#include "tev.h"

#include "kali/core/config.h"
#include "kali/core/bitmap.h"
#include "kali/core/format.h"
#include "kali/core/thread.h"

#if KALI_HAS_TEVCLIENT
#include <tevclient.h>
#endif

#include <atomic>
#include <semaphore>

namespace kali::utils {

#if KALI_HAS_TEVCLIENT

class ClientPool {
public:
    ClientPool() { tevclient::initialize(); }

    ~ClientPool()
    {
        m_clients.clear();
        tevclient::shutdown();
    }

    std::unique_ptr<tevclient::Client> acquire_client(const std::string& host, uint16_t port)
    {
        std::unique_ptr<tevclient::Client> client;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = std::find_if(
                m_clients.begin(),
                m_clients.end(),
                [&](const auto& c) { return c->getHostname() == host && c->getPort() == port; }
            );
            if (it != m_clients.end()) {
                client = std::move(*it);
                m_clients.erase(it);
            }
        }

        if (!client)
            client = std::make_unique<tevclient::Client>(host.c_str(), port);

        return client;
    }

    void release_client(std::unique_ptr<tevclient::Client> client)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_clients.push_back(std::move(client));
    }

    static ClientPool& get()
    {
        static ClientPool instance;
        return instance;
    }

private:
    std::mutex m_mutex;
    std::vector<std::unique_ptr<tevclient::Client>> m_clients;
};

#endif // KALI_HAS_TEVCLIENT

bool show_in_tev(
    const Bitmap* bitmap,
    std::optional<std::string> name,
    const std::string& host,
    uint16_t port,
    uint32_t max_retries
)
{
    KALI_CHECK_NOT_NULL(bitmap);

#if KALI_HAS_TEVCLIENT
    ref<Bitmap> converted;
    if (bitmap->component_type() != Bitmap::ComponentType::float32) {
        log_warn_once(
            "tev only supports 32-bit floating point images. Converting {} data to float.",
            bitmap->component_type()
        );
        converted = bitmap->convert(bitmap->pixel_format(), Bitmap::ComponentType::float32, false);
        bitmap = converted;
    }

    static std::atomic<uint32_t> image_counter{0};
    if (!name)
        name = fmt::format("image_{}", image_counter++);

    std::vector<const char*> channel_names(bitmap->channel_names().size());
    for (size_t i = 0; i < channel_names.size(); ++i)
        channel_names[i] = bitmap->channel_names()[i].c_str();

    for (uint32_t attempt = 1; attempt <= max_retries; ++attempt) {
        std::unique_ptr<tevclient::Client> client = ClientPool::get().acquire_client(host, port);

        if (!client->isConnected()) {
            if (client->connect() != tevclient::Error::Ok) {
                log_warn(
                    "Failed to connect to tev (attempt {}/{}): {}",
                    attempt,
                    max_retries,
                    client->lastErrorString()
                );
                continue;
            }
        }

        if (client->createImage(
                name->c_str(),
                bitmap->width(),
                bitmap->height(),
                bitmap->channel_count(),
                channel_names.data()
            )
            != tevclient::Error::Ok) {
            log_warn(
                "Failed to create image in tev (attempt {}/{}): {}",
                attempt,
                max_retries,
                client->lastErrorString()
            );
            continue;
        }

        if (client->updateImage(
                name->c_str(),
                0,
                0,
                bitmap->width(),
                bitmap->height(),
                bitmap->channel_count(),
                channel_names.data(),
                nullptr,
                nullptr,
                (const float*)bitmap->data(),
                bitmap->buffer_size() / 4
            )
            != tevclient::Error::Ok) {
            log_warn(
                "Failed to update image in tev (attempt {}/{}): {}",
                attempt,
                max_retries,
                client->lastErrorString()
            );
            continue;
        }

        ClientPool::get().release_client(std::move(client));
    }

    return true;

#else
    KALI_UNUSED(bitmap, name, host, port);
    KALI_THROW("tev support is not enabled.");
#endif
}

void show_in_tev_async(
    ref<Bitmap> bitmap,
    std::optional<std::string> name,
    const std::string& host,
    uint16_t port,
    uint32_t max_retries
)
{
    const Bitmap* bitmap_ptr = bitmap;
    bitmap_ptr->inc_ref();

    thread::do_async(
        [=]()
        {
            static std::counting_semaphore semaphore{8};
            semaphore.acquire();
            show_in_tev(bitmap_ptr, name, host, port, max_retries);
            semaphore.release();
            bitmap_ptr->dec_ref();
        }
    );
}

} // namespace kali::utils
