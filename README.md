# Midterm Project – Implement ls(1)

**Học phần:** Hệ điều hành (Operating Systems)  
**Sinh viên:** Đàm Văn Nguyên  
**Mã số sinh viên:** 24IT180  
**Tài khoản GitHub:** [DVanNguyen](https://github.com/DVanNguyen)  
**GitHub Repository:** [https://github.com/DVanNguyen/DamVanNguyen_24IT180_midterm](https://github.com/DVanNguyen/DamVanNguyen_24IT180_midterm)  

---

## 1. Giới Thiệu Tổng Quan (Overview)

Dự án này hiện thực lại công cụ dòng lệnh **`ls(1)`** của hệ điều hành UNIX từ đầu (from scratch) bằng ngôn ngữ C, tuân thủ chặt chẽ đặc tả trong tài liệu **NetBSD 10.1 General Commands Manual**.

Chương trình tương tác trực tiếp với hệ thống tệp tin (filesystem) thông qua các lời gọi hệ thống chuẩn POSIX như `opendir(3)`, `readdir(3)`, `closedir(3)`, `lstat(2)`, `stat(2)`, `readlink(2)`, `getpwuid(3)`, `getgrgid(3)`.

### Các tiêu chuẩn đạt được:
- **Thiết kế hướng module (Modular Design):** Tách bạch rõ ràng giữa phân tích đối số, đọc metadata, sắp xếp, duyệt thư mục, định dạng hiển thị và các hàm tiện ích.
- **Tuân thủ chuẩn POSIX & Hỗ trợ đa nền tảng:** Biên dịch và chạy hoàn hảo trên Linux/UNIX (môi trường chấm điểm) đồng thời có tầng trừu tượng tương thích (`compat`) để biên dịch kiểm thử trực tiếp trên Windows (MinGW/UCRT).
- **Độ tin cậy cao (Robustness):** Tuyệt đối không xảy ra hiện tượng Segmentation Fault; bắt và xử lý triệt để tất cả các trường hợp biên (edge cases) như file không tồn tại, không có quyền truy cập, symbolic link bị hỏng, phân bổ bộ nhớ động an toàn.

---

## 2. Kiến Trúc Mã Nguồn (Project Structure)

Dự án được tổ chức gọn gàng trong các thư mục `include/` (header files) và `src/` (source files):

```
.
├── Makefile                # Script biên dịch tự động bằng gcc
├── .gitignore              # Loại trừ file thực thi (ls, ls.exe) và object files (*.o)
├── README.md               # Báo cáo kỹ thuật chi tiết của đồ án
├── include/
│   ├── compat.h            # Tầng tương thích chéo POSIX và Windows
│   ├── options.h           # Định nghĩa cấu trúc cờ tùy chọn và nguyên mẫu hàm phân tích
│   ├── entry.h             # Cấu trúc FileEntry đại diện cho file/thư mục và metadata
│   ├── sort.h              # Các thuật toán sắp xếp (alphabet, thời gian, kích thước, đảo ngược)
│   ├── format.h            # Định dạng hiển thị căn lề, định dạng dài -l, inode -i, blocks -s
│   ├── list.h              # Điều phối duyệt thư mục, xử lý nhiều tham số, đệ quy -R
│   └── utils.h             # Tiện ích nối đường dẫn, humanize dung lượng -h, định dạng quyền
└── src/
    ├── main.c              # Điểm khởi đầu chương trình (main entry point)
    ├── options.c           # Phân tích cú pháp dòng lệnh, gom nhóm cờ ngắn, xử lý luật override
    ├── entry.c             # Trích xuất metadata bằng lstat/stat, đọc symlink, tính toán blocks
    ├── sort.c              # Comparator và wrapper gọi qsort chuẩn
    ├── format.c            # Tính toán độ rộng cột lớn nhất, in danh sách và dòng tổng khối (total)
    ├── list.c              # Duyệt thư mục bằng opendir/readdir, phân loại file/thư mục, đệ quy
    ├── utils.c             # Hiện thực nối chuỗi an toàn, format quyền 10 ký tự, format ngày tháng
    └── compat.c            # Hiện thực hàm tương thích hệ thống
```

---

## 3. Bảng Tính Năng & Tùy Chọn Đã Hiện Thực (Feature Matrix)

Chương trình hiện thực toàn bộ **19 tùy chọn** theo đúng mô tả của trang manual `ls [-AacdFfhiklnqRrSstuw] [file ...]`:

| Tùy chọn | Mô tả theo NetBSD Manual | Trạng thái |
| :---: | :--- | :---: |
| `-A` | Liệt kê tất cả các mục ngoại trừ `.` và `..`. | **Hoàn thành** |
| `-a` | Liệt kê tất cả các mục trong thư mục, bao gồm cả file ẩn (bắt đầu bằng dấu `.`). | **Hoàn thành** |
| `-c` | Sử dụng thời gian thay đổi trạng thái file (`ctime`) thay vì thời gian sửa đổi (`mtime`) khi sắp xếp (`-t`) hoặc in (`-l`). Ghi đè `-u`. | **Hoàn thành** |
| `-d` | Xem thư mục như file thông thường (không liệt kê nội dung bên trong). Ghi đè `-R`. | **Hoàn thành** |
| `-F` | Thêm ký tự chỉ định loại file ngay sau tên: `/` (thư mục), `*` (file thực thi), `@` (symlink), `=` (socket), `\|` (FIFO). | **Hoàn thành** |
| `-f` | Xuất kết quả không qua sắp xếp (unsorted), tự động kích hoạt `-a`. | **Hoàn thành** |
| `-h` | Hiển thị kích thước file và số block ở dạng dễ đọc cho con người (B, K, M, G, T). Ghi đè `-k`. | **Hoàn thành** |
| `-i` | In số inode (file serial number) trước mỗi mục. | **Hoàn thành** |
| `-k` | Báo cáo kích thước và khối tính theo Kilobytes (1024 bytes) cho tùy chọn `-s`. Cờ bên phải cùng giữa `-k` và `-h` sẽ có hiệu lực. | **Hoàn thành** |
| `-l` | Hiển thị định dạng dài chi tiết: chế độ file (permissions), số hard links, chủ sở hữu (owner), nhóm (group), kích thước, ngày giờ chỉnh sửa, tên file (và target nếu là symlink). | **Hoàn thành** |
| `-n` | Tương tự `-l`, nhưng hiển thị UID và GID dưới dạng số thay vì tra cứu tên owner/group. Ghi đè `-l`. | **Hoàn thành** |
| `-q` | Ép buộc in các ký tự không in được (non-printable) thành dấu `?` (mặc định khi xuất ra terminal). Ghi đè `-w`. | **Hoàn thành** |
| `-R` | Duyệt đệ quy tất cả các thư mục con gặp phải. Ghi đè `-d`. | **Hoàn thành** |
| `-r` | Đảo ngược thứ tự sắp xếp (ngược bảng chữ cái, hoặc file cũ nhất / nhỏ nhất lên đầu). | **Hoàn thành** |
| `-S` | Sắp xếp theo kích thước file, file lớn nhất lên đầu. | **Hoàn thành** |
| `-s` | Hiển thị số lượng khối hệ thống tệp tin (filesystem blocks) thực tế mà file chiếm dụng (đơn vị 512 bytes hoặc giá trị biến môi trường `BLOCKSIZE`). Khi xuất ra terminal, in dòng `total <sum>` trước danh sách. | **Hoàn thành** |
| `-t` | Sắp xếp theo thời gian sửa đổi gần nhất lên trước (nếu kết hợp `-c` dùng ctime, `-u` dùng atime). | **Hoàn thành** |
| `-u` | Sử dụng thời gian truy cập gần nhất (`atime`) thay vì mtime cho việc sắp xếp (`-t`) hoặc in (`-l`). Ghi đè `-c`. | **Hoàn thành** |
| `-w` | Ép buộc in nguyên bản (raw) các ký tự không in được (mặc định khi chuyển hướng pipe/file). Ghi đè `-q`. | **Hoàn thành** |

### Quy Tắc Ghi Đè (Override Rules) Được Hiện Thực:
- `-w` và `-q`: Cờ xuất hiện sau cùng trên dòng lệnh sẽ quyết định cách hiển thị ký tự đặc biệt.
- `-l` và `-n`: Cờ xuất hiện sau cùng quyết định dạng hiển thị tên hay ID số.
- `-c` và `-u`: Cờ xuất hiện sau cùng quyết định trường thời gian sử dụng (`ctime` hay `atime`).
- `-R` và `-d`: Cờ xuất hiện sau cùng quyết định duyệt đệ quy hay xem thư mục như file.
- `-k` và `-h`: Cờ bên phải cùng sẽ ghi đè cờ bên trái cho đơn vị khối `-s`.

---

## 4. Hướng Dẫn Biên Dịch & Chạy Chương Trình

### 4.1. Yêu cầu hệ thống
- Trình biên dịch C: `gcc` hoặc `clang` hỗ trợ chuẩn C99 trở lên.
- Công cụ build: `make` (trên Linux/macOS) hoặc `mingw32-make` (trên Windows qua MSYS2/MinGW).

### 4.2. Lệnh biên dịch
Để biên dịch chương trình với các cờ kiểm tra nghiêm ngặt (`-Wall -Wextra -pedantic -std=c99`):
```bash
# Trên Linux / macOS:
make

# Trên Windows (sử dụng MinGW):
mingw32-make
```
Sau khi biên dịch thành công, file thực thi `ls` (hoặc `ls.exe` trên Windows) sẽ được tạo ra tại thư mục gốc của dự án.

### 4.3. Dọn dẹp bản build
Để xóa sạch các file đối tượng (`.o`) và file thực thi:
```bash
# Trên Linux / macOS:
make clean

# Trên Windows:
mingw32-make clean
```

---

## 5. Ví Dụ Sử Dụng & Kết Quả Chạy Thử Thực Tế

### 5.1. Liệt kê mặc định (Default Listing)
Mặc định mỗi tệp hiển thị trên một dòng, được sắp xếp theo thứ tự từ điển:
```bash
$ ./ls
Makefile
include
src
```

### 5.2. Liệt kê tất cả file bao gồm file ẩn (`-a` và `-A`)
```bash
$ ./ls -a
.
..
.gitignore
Makefile
include
src

$ ./ls -A
.gitignore
Makefile
include
src
```

### 5.3. Định dạng dài chi tiết (`-l`) kèm dung lượng dễ đọc (`-h`)
Hiển thị đầy đủ quyền 10 ký tự, số liên kết, owner, group, kích thước căn lề tự động, ngày giờ và tên:
```bash
$ ./ls -lh
total 296K
-rw-rw-rw- 1 Nguyen  0  591B Oct  2 16:17 Makefile
drwxrwxrwx 1 Nguyen  0  4.0K Oct  2 16:17 include
drwxrwxrwx 1 Nguyen  0  4.0K Oct  2 16:19 src
```

### 5.4. Hiển thị dạng số UID/GID (`-n`)
```bash
$ ./ls -n
total 592
-rw-rw-rw- 1 0  0     591 Oct  2 16:17 Makefile
drwxrwxrwx 1 0  0    4096 Oct  2 16:17 include
drwxrwxrwx 1 0  0    4096 Oct  2 16:19 src
```

### 5.5. Hiển thị Inode (`-i`) và Khối tệp tin (`-s`)
```bash
$ ./ls -lis
total 592
0   2 -rw-rw-rw- 1 Nguyen  0     591 Oct  2 16:17 Makefile
0   8 drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:17 include
0   8 drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:19 src
```

### 5.6. Phân loại loại file với cờ (`-F`)
Thêm dấu `/` cho thư mục, `*` cho file thực thi:
```bash
$ ./ls -F
Makefile
include/
src/
```

### 5.7. Các chế độ sắp xếp (`-S`, `-t`, `-r`)
- Sắp xếp theo dung lượng giảm dần:
  ```bash
  $ ./ls -lS
  ```
- Sắp xếp theo thời gian mới nhất:
  ```bash
  $ ./ls -lt
  ```
- Đảo ngược thứ tự sắp xếp:
  ```bash
  $ ./ls -lr
  ```

### 5.8. Xem thư mục như file (`-d`) và Duyệt đệ quy (`-R`)
- Liệt kê chính thư mục mà không đi vào trong:
  ```bash
  $ ./ls -ld include
  drwxrwxrwx 1 Nguyen  0  4096 Oct  2 16:17 include
  ```
- Duyệt đệ quy toàn bộ cây thư mục con:
  ```bash
  $ ./ls -R include
  include:
  compat.h
  entry.h
  format.h
  list.h
  options.h
  sort.h
  utils.h
  ```

---

## 6. Xử Lý Các Trường Hợp Đặc Biệt (Edge Cases & Robustness)

1. **Nhiều đối số (Multiple Operands):**
   - Tuân thủ quy định NetBSD: Các đối số không phải thư mục (files) luôn được hiển thị trước; các đối số là thư mục được hiển thị sau và có in tiêu đề `dir_name:`.
   - Giữa các khối thư mục có dấu ngắt dòng cách nhau rõ ràng.

2. **File hoặc thư mục không tồn tại:**
   - Khi gặp file không tồn tại (ví dụ: `./ls file_that_does_not_exist existing_file`), chương trình in thông báo lỗi chuẩn ra `stderr`:  
     `ls: file_that_does_not_exist: No such file or directory`  
   - Chương trình **không bị dừng hay crash** mà tiếp tục xử lý chính xác các file hợp lệ còn lại, và kết thúc với mã thoát lỗi `> 0`.

3. **Tùy chọn không hợp lệ (Unknown Flags):**
   - Khi người dùng nhập cờ sai (ví dụ: `./ls -z`), chương trình báo lỗi `ls: unknown option -- z`, hiển thị cú pháp sử dụng (`usage`) và thoát với mã lỗi 1.

4. **Ký tự ngăn cách cờ (`--`):**
   - Hỗ trợ ký tự `--` để kết thúc danh sách cờ, cho phép xử lý an toàn các file có tên bắt đầu bằng dấu gạch ngang (ví dụ: `./ls -- -filename`).

5. **Biến môi trường `BLOCKSIZE`:**
   - Hỗ trợ đọc biến môi trường `BLOCKSIZE` để điều chỉnh đơn vị khối cho tùy chọn `-s`.

---

## 7. Hướng Dẫn Đẩy Lên GitHub (Git Submission Instructions)

Repository đã được thiết lập với `.gitignore` nhằm loại bỏ hoàn toàn các file nhị phân và object files. Để đẩy dự án lên GitHub cá nhân:

```bash
# 1. Khởi tạo kho git (nếu chưa khởi tạo)
git init

# 2. Thêm tất cả mã nguồn, Makefile, README.md, .gitignore
git add include/ src/ Makefile .gitignore README.md

# 3. Tạo commit đầu tiên
git commit -m "Initial commit: complete modular implementation of ls(1) per NetBSD manual"

# 4. Đặt nhánh chính là main
git branch -M main

# 5. Liên kết với repository trên GitHub của bạn
git remote add origin https://github.com/DVanNguyen/DamVanNguyen_24IT180_midterm.git

# 6. Đẩy code lên GitHub
git push -u origin main
```
