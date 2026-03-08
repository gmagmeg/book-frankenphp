# Geminiエージェントへの指示
*   すべての回答は日本語で行うこと。

---

# Gemini Project Overview: Re:VIEW Template


## 執筆内容
- アプリケーションを作成しながら、FraneknPHPの概要を学ぶ。FrankenPHP入門、導入検討しているユーザーに向けた内容
- 作成アプリケーション内容
  - ユーザーからリクエストを受けると、バックグラウンドでCSVの生成が始まる
  - CSVの生成が終わると、ユーザー側にワンタイムパスワード付きのpush通知が飛ぶ
  - ユーザーはpush通知を受け取り、ワンタイムパスワードが一致すれば、CSVをダウンロードすることができる


## Directory Overview

This directory contains a template for writing and publishing books using the Re:VIEW (version 5.x) typesetting system. It is maintained by TechBooster and is primarily configured for creating technical books, with support for various output formats including PDF, EPUB, and Web (HTML).

The project uses a combination of Ruby (for the core Re:VIEW engine) and Node.js (for task automation) to build the final documents.

## Key Files

*   `articles/`: This is the central directory where all book content and configuration reside.
    *   `*.re`: These are the source files for the book's content, written in the Re:VIEW markup format.
        *   `preface.re`: The book's preface.
        *   `article.re`: The main sample chapter.
    *   `catalog.yml`: This file defines the structure of the book by listing the `.re` files in the desired order (preface, chapters, appendix, etc.).
    *   `config.yml`: The main configuration file for the Re:VIEW build process. It contains metadata like the book title, author, and publisher, and controls technical aspects of the output such as paper size, styling, and versioning.
    *   `*.scss`/`*.css`: SASS/CSS files used to style the EPUB and Web outputs.
*   `Gemfile`: Defines the project's Ruby dependencies, most importantly `review` (the typesetting engine) and `rake` (the task runner).
*   `package.json`: Defines the project's Node.js dependencies and build scripts. It uses `grunt` to automate the build process.
*   `Gruntfile.js`: The Grunt configuration file. It defines the sequence of shell commands (`review-preproc`, `review-compile`, `rake`) needed to convert the `.re` source files into the various output formats.
*   `build-in-docker.sh`: A shell script to build the project inside a pre-configured Docker container, simplifying the setup of the required TeX environment.

## Building and Running

### 1. Initial Setup

First, install the necessary Ruby and Node.js dependencies.

```bash
# Install Ruby gems (requires Bundler)
gem install bundler
bundle install

# Install Node.js packages
npm install
```

### 2. Building the Book

The project provides `npm` scripts as convenient shortcuts for building the different output formats. All build commands should be run from the root of the project directory.

*   **To generate a PDF:**
    ```bash
    npm run pdf
    ```

*   **To generate an EPUB:**
    ```bash
    npm run epub
    ```

*   **To generate a multi-page HTML version (for web):**
    ```bash
    npm run web
    ```

*   **To generate plain text files:**
    ```bash
    npm run text
    ```

### Using Docker

For users who do not want to install a local TeX distribution and other dependencies, a Docker-based build is available.

```bash
./build-in-docker.sh
```

## Development Conventions

*   **Book Content:** All manuscript content is written in Re:VIEW markup and stored in `.re` files within the `articles/` directory.
*   **Book Structure:** The order and inclusion of chapters are managed in `articles/catalog.yml`.
*   **Output Configuration:** To change settings like paper size (e.g., from B5 to A5) or media type (e.g., `print` vs. `ebook`), you must edit the `texdocumentclass` parameter in `articles/config.yml`. The file contains commented-out examples for common configurations.
*   **Styling:** Web and EPUB styling can be modified by editing the `.scss` files in the `articles/` directory and then recompiling the CSS with the following command:
    ```bash
    ./rebuild-css.sh
    ```
