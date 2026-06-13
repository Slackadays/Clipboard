# Clipboard 项目 Bug 修复记录

以下是我在项目目录中发现并修复的 Bug 列表。

## Bug 1: PipeFd 移动赋值操作符问题

- **文件**: `src/cbwayland/src/fd.cpp`
- **问题**: `PipeFd::operator=` 方法错误地显式调用了析构函数 (`PipeFd::~PipeFd();`) 而不是在交换文件描述符之前正确关闭它们。
- **修复**: 替换为正确的关闭操作：
  ```cpp
  PipeFd& PipeFd::operator=(PipeFd&& other) noexcept {
      closeRead();
      closeWrite();
      std::swap(m_readFd, other.m_readFd);
      std::swap(m_writeFd, other.m_writeFd);
      return *this;
  }
  ```

## Bug 2: 缺失 SSIZE_MAX 定义

- **文件**: `src/cbwayland/src/fd.cpp`
- **问题**: `FdBuffer::constrainSize` 函数使用了 `SSIZE_MAX` 但未包含定义它的头文件。
- **修复**: 添加缺失的头文件包含：
  ```cpp
  #include <sys/types.h>
  #include <limits.h>  // 添加此行
  ```

## Bug 3: Note 功能未正确处理尾部换行符

- **文件**: `src/cb/src/actions/note.cpp`
- **问题**: `notePipe` 函数保留了管道输入中的尾部换行符，导致测试失败。当用户通过 `echo "Foobar" | cb note` 管道输入时，测试期望的note内容是 "Foobar"（不含换行），但实际保存的是 "Foobar\n"。
- **修复**: 添加尾部换行符和回车符的去除逻辑：
  ```cpp
  void notePipe() {
      std::string content(pipedInContent());
      // 去除尾部的换行符和回车符
      while (!content.empty() && 
             (content.back() == '\n' || content.back() == '\r')) {
          content.pop_back();
      }
      writeToFile(path.metadata.notes, content);
      if (output_silent || confirmation_silent) return;
      stopIndicator();
      fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Saved note \"%s\"[blank]\n").data(), content.data());
      exit(EXIT_SUCCESS);
  }
  ```

所有修复后，项目能够成功编译并通过所有测试用例。