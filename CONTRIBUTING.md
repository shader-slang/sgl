# Shader-Slang Open Source Project

## Contribution Guide

Thank you for considering contributing to the Shader-Slang project! We welcome your help to improve and enhance our project. Please take a moment to read through this guide to understand how you can contribute.

This document is designed to guide you in contributing to the project. It is intended to be easy to follow without sending readers to other pages and links. You can simply copy and paste the command lines described in this document.

* Contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant the rights to use your contribution.
* When you submit a pull request, a CLA bot will determine whether you need to sign a CLA. Simply follow the instructions provided.
* Please read and follow the contributor Code of Conduct.
* Bug reports and feature requests should be submitted via the GitHub issue tracker.
* Changes should ideally come in as small pull requests on top of master, coming from your own personal fork of the project.
* Large features that will involve multiple contributors or a long development time should be discussed in issues and broken down into smaller pieces that can be implemented and checked in stages.

## Table of Contents

1. Contribution Process  
   * Forking the Repository  
   * Cloning Your Fork  
   * Creating a Branch  
   * Build Slang from Source  
   * Making Changes  
   * Testing  
   * Commit to the Branch  
   * Push to Forked Repository
2. Pull Request  
   * Addressing Code Reviews  
   * Labeling Breaking Changes  
   * Source Code Formatting  
   * Document Changes
3. Code Style
4. Issue Tracking
5. Communication
6. License

## Contribution Process

### Forking the Repository

Navigate to the Shader-Slang repository. Click on the "Fork" button in the top right corner to create a copy of the repository in your GitHub account. Make sure your "Actions" are enabled. Visit your forked repository, click on the "Actions" tab, and enable the actions.

### Cloning Your Fork

1. Clone your fork locally, replacing "USER-NAME" in the command below with your actual username.  
```  
$ git clone --recursive --tags https://github.com/USER-NAME/sgl.git  
$ cd sgl  
```
2. Fetch tags by adding the original repository as an upstream. It is important to have tags in your forked repository because our workflow/action uses the information for the build process. But the tags are not fetched by default when you fork a repository in GitHub. You need to add the original repository as an upstream and fetch tags manually.  
```  
$ git remote add upstream https://github.com/shader-slang/sgl.git  
$ git fetch --tags upstream  
```  
You can check whether the tags are fetched properly with the following command.  
```  
$ git tag -l  
```
3. Push tags to your forked repository. The tags are fetched to your local machine but haven't been pushed to the forked repository yet. You need to push tags to your forked repository with the following command.  
```  
$ git push --tags origin  
```

### Creating a Branch

Create a new branch for your contribution:

```
$ git checkout -b feature/your-feature-name
```

### Build SGL from Source

Please follow the instructions in the README.md on how to build SGL from source.

For a quick reference, follow the instructions below.

#### Windows

Download and install CMake from CMake.org/download.

Run CMake with the following command to generate a Visual Studio 2022 Solution:

```
C:\git\sgl> cmake.exe --preset vs2022 # For Visual Studio 2022
C:\git\sgl> cmake.exe --preset vs2019 # For Visual Studio 2019
```

Open `build/sgl.sln` with Visual Studio IDE and build it for "x64".

Or you can build with the following command:

```
C:\git\sgl> cmake.exe --build --preset release
```

#### Linux

Install CMake and Ninja.

```
$ sudo apt-get install cmake ninja-build
```

Run CMake with the following command to generate Makefile:

```
$ cmake --preset default
```

Build with the following command:

```
$ cmake --build --preset release
```

#### MacOS

Install Xcode from the App Store.

Install CMake and Ninja; we recommend using Homebrew for installing them.

```
$ brew install ninja
$ brew install cmake
```

Run CMake with the following command to generate Makefile:

```
$ cmake --preset default
```

Build with the following command:

```
$ cmake --build --preset release
```

### Making Changes

Make your changes to the codebase following our coding conventions. Implement your feature or bug fix, keeping in mind the scope of the issue you're addressing.

### Testing

Before submitting your changes, make sure to run the tests to ensure that your changes do not break existing functionality.

Run the test suite with the following command:

```
$ ctest --test-dir build
```

### Commit to the Branch

Commit your changes to the branch with a descriptive commit message:

```
$ git commit
```

It is important to have a descriptive commit message. Unlike comments inside the source code, the commit messages don't spoil over time because they are tied to specific changes and can be reviewed by many people many years later.

Here is a good example of a commit message:

> Add user authentication feature
> 
> Fixes #1234
> 
> This commit introduces a new user authentication feature. It includes changes to the login page, user database, and session management to provide secure user authentication.

### Push to Forked Repository

Push your branch to your forked repository with the following command:

```
$ git push origin feature/your-feature-name
```

After the changes are pushed to your forked repository, the change needs to be merged to the final destination `shader-slang/sgl`. In order to proceed, you will need to create a "Pull Request," or "PR" for short.

When you push to your forked repository, `git push` usually prints a URL that allows you to create a PR.

If you missed a chance to use the URL, you can still create a PR from the GitHub webpage. Go to your forked repository and change the branch name to the one you used for `git push`. It will show a message like "This branch is 1 commit ahead of `shader-slang/sgl:master`." You can create a PR by clicking on the message.

## Pull Request

Once a PR is created against `shader-slang/sgl:master`, the PR will be merged when the following conditions are met:

1. The PR is reviewed and got approval.
2. All of the workflows pass.

When the conditions above are all met, you will have a chance to rewrite the commit message. Since the SGL repo uses the "squash" strategy for merging, multiple commits in your PR will become one commit. By default, GitHub will concatenate all of the commit messages sequentially, but often it is not readable. Please rewrite the final commit message in a way that people can easily understand what the purpose of the commit is.

There are two cases where the workflow may fail for reasons that are not directly related to the change:

1. "Breaking change" labeling is missing.
2. Source code "Format" needs to be changed.

### Addressing Code Reviews

After your pull request is created, you will receive code reviews from the community within 24 hours.

The PR requires approval from people who have permissions. They will review the changes before approving the pull. During this step, you will get feedback from other people, and they may request you to make some changes.

Follow-up changes that address review comments should be pushed to your pull request branch as additional commits. Any additional commits made to the same branch in your forked repository will show up on the PR as incremental changes.

When your branch is out of sync with top-of-tree, submit a merge commit to keep them in sync. Do not rebase and force push after the PR is created to keep the change history during the review process.

Use these commands to sync your branch:

```
$ git fetch upstream master
$ git merge upstream/master # resolve any conflicts here
$ git submodule update --recursive
```

The SGL repository uses the squash strategy for merging pull requests, which means all your commits will be squashed into one commit by GitHub upon merge.

### Labeling Breaking Changes

All pull requests must be labeled as either `pr: non-breaking` or `pr: breaking change` before it can be merged to the main branch. If you are already a committer, you are expected to label your PR when you create it. If you are not yet a committer, a reviewer will do this for you.

A PR is considered to introduce a breaking change if an existing application that uses SGL may no longer compile or behave the same way with the change. Typical examples of breaking changes include:

* Changes to the public API that modify behavior in a way that breaks binary compatibility.
* Changes to the language syntax or semantics that may cause existing code to not compile or produce different run-time results.
* Removing or renaming existing functionality.

### Source Code Formatting

When the PR contains source code changes, one of the workflows will check the formatting of the code.

Code formatting can be automatically fixed on your branch by commenting `/format`; a bot will proceed to open a PR targeting _your_ branch. You can merge the generated PR into your branch, and the problem will be resolved.

### Document Changes

When the PR contains document changes, make sure to update all relevant documentation. Documentation updates should be included in the same PR as the code changes they document.

## Code Style

Follow our Coding Conventions to maintain consistency throughout the project.

Here are a few highlights:

1. Indent by four spaces. Don't use tabs except in files that require them (e.g., Makefiles).
2. Use consistent naming conventions throughout the codebase.
3. Types should use UpperCamelCase, values should use lowerCamelCase, and macros should use `SCREAMING_SNAKE_CASE` with a prefix `SGL_`.
4. Member variables should use a consistent prefix (e.g., `m_` for class member variables).
5. Write clear, descriptive comments that explain the "why" of your code more than the "what."
6. Follow the existing patterns in the codebase for consistency.

## Issue Tracking

We track all our work with GitHub issues. Check the Issues for open issues. If you find a bug or want to suggest an enhancement, please open a new issue.

If you're new to the project or looking for a good starting point, consider exploring issues labeled as "good first issue". These are beginner-friendly bugs that provide a great entry point for new contributors.

## Communication

Join our GitHub Discussions and Discord server for questions and community support.

## License

By contributing to SGL, you agree that your contributions will be licensed under the Apache License 2.0 with LLVM Exception. The full text of the License can be found in the LICENSE file in the root of the repository.
