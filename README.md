# Midterm Project – Implement ls(1)

**Học phần:** Lập trình hệ thống  
**Sinh viên:** Đàm Văn Nguyên  
**Mã số sinh viên:** 24IT180  

---

## 1. Giới Thiệu Tổng Quan

Dự án này hiện thực lại công cụ dòng lệnh **`ls(1)`** của hệ điều hành UNIX từ đầu (from scratch) bằng ngôn ngữ C, tuân thủ chặt chẽ đặc tả trong tài liệu **NetBSD 10.1 General Commands Manual**.

Chương trình tương tác trực tiếp với hệ thống tệp tin (filesystem) thông qua các lời gọi hệ thống chuẩn POSIX như `opendir(3)`, `readdir(3)`, `closedir(3)`, `lstat(2)`, `stat(2)`, `readlink(2)`, `getpwuid(3)`, `getgrgid(3)`.

### Các tiêu chuẩn đạt được:
- **Thiết kế hướng module (Modular Design):** Tách bạch rõ ràng giữa phân tích đối số, đọc metadata, sắp xếp, duyệt thư mục, định dạng hiển thị và các hàm tiện ích.
- **Tuân thủ chuẩn POSIX & Hỗ trợ đa nền tảng:** Biên dịch và chạy hoàn hảo trên Linux/UNIX (môi trường chấm điểm) đồng thời có tầng trừu tượng tương thích (`compat`) để biên dịch kiểm thử trực tiếp trên Windows (MinGW/UCRT).
- **Độ tin cậy cao (Robustness):** Tuyệt đối không xảy ra hiện tượng Segmentation Fault; bắt và xử lý triệt để tất cả các trường hợp biên (edge cases) như file không tồn tại, không có quyền truy cập, symbolic link bị hỏng, phân bổ bộ nhớ động an toàn.

---

## 2. Kiến Trúc Mã Nguồn 

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

## 3. Bảng Tính Năng & Tùy Chọn Đã Hiện Thực

| Tùy chọn | Mô tả theo NetBSD Manual | Trạng thái |
| :---: | :--- | :---: |
| `-A` | `[should_include_entry()]` Liệt kê tất cả các mục ngoại trừ `.` và `..`. | **Hoàn thành** |
| `-a` | `[should_include_entry()]` Liệt kê tất cả các mục trong thư mục, bao gồm cả file ẩn (bắt đầu bằng dấu `.`). | **Hoàn thành** |
| `-c` | `[entry_create(), sort_entries()]` Sử dụng thời gian thay đổi trạng thái file (`ctime`) thay vì thời gian sửa đổi (`mtime`) khi sắp xếp (`-t`) hoặc in (`-l`). Ghi đè `-u`. | **Hoàn thành** |
| `-d` | `[list_operands()]` Xem thư mục như file thông thường (không liệt kê nội dung bên trong). Ghi đè `-R`. | **Hoàn thành** |
| `-F` | `[utils_get_classifier()]` Thêm ký tự chỉ định loại file ngay sau tên: `/` (thư mục), `*` (file thực thi), `@` (symlink), `=` (socket), `\|` (FIFO). | **Hoàn thành** |
| `-f` | `[options_parse(), sort_entries()]` Xuất kết quả không qua sắp xếp (unsorted), tự động kích hoạt `-a`. | **Hoàn thành** |
| `-h` | `[utils_humanize_size()]` Hiển thị kích thước file và số block ở dạng dễ đọc cho con người (B, K, M, G, T). Ghi đè `-k`. | **Hoàn thành** |
| `-i` | `[format_print_entries()]` In số inode (file serial number) trước mỗi mục. | **Hoàn thành** |
| `-k` | `[entry_create(), format_print_entries()]` Báo cáo kích thước và khối tính theo Kilobytes (1024 bytes) cho tùy chọn `-s`. Cờ bên phải cùng giữa `-k` và `-h` sẽ có hiệu lực. | **Hoàn thành** |
| `-l` | `[format_print_entries(), utils_format_mode()]` Hiển thị định dạng dài chi tiết: chế độ file (permissions), số hard links, chủ sở hữu (owner), nhóm (group), kích thước, ngày giờ chỉnh sửa, tên file (và target nếu là symlink). | **Hoàn thành** |
| `-n` | `[entry_create(), format_print_entries()]` Tương tự `-l`, nhưng hiển thị UID và GID dưới dạng số thay vì tra cứu tên owner/group. Ghi đè `-l`. | **Hoàn thành** |
| `-q` | `[utils_sanitize_name()]` Ép buộc in các ký tự không in được (non-printable) thành dấu `?` (mặc định khi xuất ra terminal). Ghi đè `-w`. | **Hoàn thành** |
| `-R` | `[list_directory()]` Duyệt đệ quy tất cả các thư mục con gặp phải. Ghi đè `-d`. | **Hoàn thành** |
| `-r` | `[sort_entries(), entry_cmp()]` Đảo ngược thứ tự sắp xếp (ngược bảng chữ cái, hoặc file cũ nhất / nhỏ nhất lên đầu). | **Hoàn thành** |
| `-S` | `[sort_entries(), entry_cmp()]` Sắp xếp theo kích thước file, file lớn nhất lên đầu. | **Hoàn thành** |
| `-s` | `[compat_get_blocks(), format_print_total()]` Hiển thị số lượng khối hệ thống tệp tin (filesystem blocks) thực tế mà file chiếm dụng (đơn vị 512 bytes hoặc giá trị biến môi trường `BLOCKSIZE`). Khi xuất ra terminal, in dòng `total <sum>` trước danh sách. | **Hoàn thành** |
| `-t` | `[sort_entries(), entry_cmp()]` Sắp xếp theo thời gian sửa đổi gần nhất lên trước (nếu kết hợp `-c` dùng ctime, `-u` dùng atime). | **Hoàn thành** |
| `-u` | `[entry_create(), sort_entries()]` Sử dụng thời gian truy cập gần nhất (`atime`) thay vì mtime cho việc sắp xếp (`-t`) hoặc in (`-l`). Ghi đè `-c`. | **Hoàn thành** |
| `-w` | `[utils_sanitize_name()]` Ép buộc in nguyên bản (raw) các ký tự không in được (mặc định khi chuyển hướng pipe/file). Ghi đè `-q`. | **Hoàn thành** |

### Quy Tắc Ghi Đè (Override Rules) Được Hiện Thực:
- `-w` và `-q`: Cờ xuất hiện sau cùng trên dòng lệnh sẽ quyết định cách hiển thị ký tự đặc biệt.
- `-l` và `-n`: Cờ xuất hiện sau cùng quyết định dạng hiển thị tên hay ID số.
- `-c` và `-u`: Cờ xuất hiện sau cùng quyết định trường thời gian sử dụng (`ctime` hay `atime`).
- `-R` và `-d`: Cờ xuất hiện sau cùng quyết định duyệt đệ quy hay xem thư mục như file.
- `-k` và `-h`: Cờ bên phải cùng sẽ ghi đè cờ bên trái cho đơn vị khối `-s`.

---

## 4. Các Bước Tải Về Và Kiểm Thử

Quy trình chi tiết và chạy kiểm thử toàn bộ các tính năng:

### 4.1. Bước 1: Tải mã nguồn từ GitHub
```bash
git clone https://github.com/DVanNguyen/DamVanNguyen_24IT180_midterm.git
cd DamVanNguyen_24IT180_midterm
```

### 4.2. Bước 2: Biên dịch mã nguồn
Dự án được cấu hình `Makefile` tự động nhận diện môi trường và hỗ trợ cả Linux/macOS lẫn Windows:
```bash
# Trên hệ điều hành Linux / macOS:
make

# Trên hệ điều hành Windows (sử dụng MinGW / MSYS2):
mingw32-make
```
*Kết quả:* Trình biên dịch tạo file thực thi `./ls` (hoặc `ls.exe` trên Windows) với các cờ kiểm tra nghiêm ngặt `-Wall -Wextra -pedantic -std=c99` mà không phát sinh bất kỳ cảnh báo (warning) hay lỗi nào.

### 4.3. Bước 3: Dọn dẹp bản build (Clean)
Khi cần xóa sạch tất cả file đối tượng (`.o`) và file thực thi nhị phân để trả về trạng thái mã nguồn ban đầu:
```bash
# Trên Linux / macOS:
make clean

# Trên Windows:
mingw32-make clean
```

---

## 5. Kịch Bản Kiểm Thử & Kết Quả Chạy Thực Tế Đầy Đủ

Dưới đây là tập hợp đầy đủ các lệnh chạy kiểm thử cho từng nhóm tính năng cùng kết quả xuất thực tế của chương trình:

### 5.1. Liệt kê mặc định (Default Listing)
Mặc định mỗi tệp hiển thị trên một dòng, được sắp xếp theo thứ tự từ điển:
```bash
$ ./ls
Makefile
README.md
include
ls
src
```

### 5.2. Liệt kê file ẩn (`-a` và `-A`)
- Cờ `-a` liệt kê toàn bộ file ẩn, bao gồm `.` và `..`:
```bash
$ ./ls -a
.
..
.git
.gitignore
Makefile
README.md
include
ls
src
```
- Cờ `-A` liệt kê file ẩn nhưng loại trừ `.` và `..`:
```bash
$ ./ls -A
.git
.gitignore
Makefile
README.md
include
ls
src
```

### 5.3. Định dạng dài chi tiết (`-l`) kết hợp dung lượng dễ đọc (`-h`) và UID/GID số (`-n`)
- Cờ `-l` hiển thị 10 ký tự quyền (file mode), số links, owner, group, kích thước căn lề, ngày giờ và tên:
```bash
$ ./ls -l
total 619
-rw-rw-rw- 1 Nguyen  0     594 Oct  2 16:23 Makefile
-rw-rw-rw- 1 Nguyen  0   13796 Oct  2 16:55 README.md
drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:17 include
-rwxrwxrwx 1 Nguyen  0  293666 Oct  7 22:57 ls
drwxrwxrwx 1 Nguyen  0    4096 Oct  7 22:57 src
```
- Cờ `-lh` chuyển kích thước sang đơn vị dễ đọc (B, K, M, G):
```bash
$ ./ls -lh
total 310K
-rw-rw-rw- 1 Nguyen  0  594B Oct  2 16:23 Makefile
-rw-rw-rw- 1 Nguyen  0   14K Oct  2 16:55 README.md
drwxrwxrwx 1 Nguyen  0  4.0K Oct  2 16:17 include
-rwxrwxrwx 1 Nguyen  0  287K Oct  7 22:57 ls
drwxrwxrwx 1 Nguyen  0  4.0K Oct  7 22:57 src
```
- Cờ `-n` hiển thị UID và GID dạng số:
```bash
$ ./ls -n
total 619
-rw-rw-rw- 1 0  0     594 Oct  2 16:23 Makefile
-rw-rw-rw- 1 0  0   13796 Oct  2 16:55 README.md
drwxrwxrwx 1 0  0    4096 Oct  2 16:17 include
-rwxrwxrwx 1 0  0  293666 Oct  7 22:57 ls
drwxrwxrwx 1 0  0    4096 Oct  7 22:57 src
```

### 5.4. Hiển thị Inode (`-i`), Khối tệp (`-s`, `-sk`) và kết hợp (`-lis`)
- In số inode (`-i`):
```bash
$ ./ls -i
0 Makefile
0 README.md
0 include
0 ls
0 src
```
- In số khối hệ thống tệp chiếm dụng (`-s`):
```bash
$ ./ls -s
  2 Makefile
 27 README.md
  8 include
574 ls
  8 src
```
- Báo cáo số khối theo đơn vị Kilobytes (`-sk`):
```bash
$ ./ls -sk
  1 Makefile
 14 README.md
  4 include
287 ls
  4 src
```
- Kết hợp định dạng dài, inode và số block (`-lis`):
```bash
$ ./ls -lis
total 619
0   2 -rw-rw-rw- 1 Nguyen  0     594 Oct  2 16:23 Makefile
0  27 -rw-rw-rw- 1 Nguyen  0   13796 Oct  2 16:55 README.md
0   8 drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:17 include
0 574 -rwxrwxrwx 1 Nguyen  0  293666 Oct  7 22:57 ls
0   8 drwxrwxrwx 1 Nguyen  0    4096 Oct  7 22:57 src
```

### 5.5. Phân loại loại tệp tin với ký hiệu (`-F`)
Thêm dấu `/` cho thư mục, `*` cho tệp thực thi:
```bash
$ ./ls -F
Makefile
README.md
include/
ls*
src/
```

### 5.6. Các chế độ sắp xếp (`-S`, `-t`, `-r`, `-f`)
- Sắp xếp theo dung lượng giảm dần (`-lS`):
```bash
$ ./ls -lS
total 619
-rwxrwxrwx 1 Nguyen  0  293666 Oct  7 22:57 ls
-rw-rw-rw- 1 Nguyen  0   13796 Oct  2 16:55 README.md
drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:17 include
drwxrwxrwx 1 Nguyen  0    4096 Oct  7 22:57 src
-rw-rw-rw- 1 Nguyen  0     594 Oct  2 16:23 Makefile
```
- Sắp xếp theo thời gian sửa đổi gần nhất lên đầu (`-lt`):
```bash
$ ./ls -lt
total 619
-rwxrwxrwx 1 Nguyen  0  293666 Oct  7 22:57 ls
drwxrwxrwx 1 Nguyen  0    4096 Oct  7 22:57 src
-rw-rw-rw- 1 Nguyen  0   13796 Oct  2 16:55 README.md
-rw-rw-rw- 1 Nguyen  0     594 Oct  2 16:23 Makefile
drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:17 include
```
- Đảo ngược thứ tự sắp xếp (`-lr`):
```bash
$ ./ls -lr
total 619
drwxrwxrwx 1 Nguyen  0    4096 Oct  7 22:57 src
-rwxrwxrwx 1 Nguyen  0  293666 Oct  7 22:57 ls
drwxrwxrwx 1 Nguyen  0    4096 Oct  2 16:17 include
-rw-rw-rw- 1 Nguyen  0   13796 Oct  2 16:55 README.md
-rw-rw-rw- 1 Nguyen  0     594 Oct  2 16:23 Makefile
```
- Xuất dữ liệu không qua sắp xếp (`-f`):
```bash
$ ./ls -f
.
..
.git
.gitignore
include
ls
Makefile
README.md
src
```

### 5.7. Xem thư mục như file (`-d`) và Duyệt đệ quy (`-R`)
- Liệt kê thông tin chính thư mục mà không duyệt bên trong (`-d`, `-ld`):
```bash
$ ./ls -ld include
drwxrwxrwx 1 Nguyen  0  4096 Oct  2 16:17 include
```
- Duyệt đệ quy toàn bộ cây thư mục con (`-R`):
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

### 5.8. Xử lý nhiều đối số (Multiple Operands)
Hiển thị các tệp không phải thư mục trước, các thư mục hiển thị sau và có in tiêu đề phân cách:
```bash
$ ./ls Makefile include
Makefile

include:
compat.h
entry.h
format.h
list.h
options.h
sort.h
utils.h
```

### 5.9. Kiểm thử xử lý lỗi và các trường hợp biên
- Khi gặp tệp không tồn tại: In lỗi ra `stderr`, tiếp tục xử lý các tệp hợp lệ khác và trả về mã thoát lỗi `> 0`:
```bash
$ ./ls file_khong_ton_tai.txt Makefile
ls: file_khong_ton_tai.txt: No such file or directory
Makefile
```
- Khi nhập cờ tùy chọn không hợp lệ: Báo lỗi cờ không xác định, hiển thị cú pháp và thoát:
```bash
$ ./ls -z
ls: unknown option -- z
usage: ls [-AacdFfhiklnqRrSstuw] [file ...]
```

---

## 7. Thông Tin Nộp Bài & Liên Kết GitHub

Dự án đã được lưu trữ và quản lý phiên bản hoàn chỉnh trên GitHub theo đúng quy cách của đề bài:

- **Họ và tên:** Đàm Văn Nguyên
- **Mã số sinh viên:** 24IT180
- **Đường dẫn GitHub (Repository URL):** [https://github.com/DVanNguyen/DamVanNguyen_24IT180_midterm](https://github.com/DVanNguyen/DamVanNguyen_24IT180_midterm)
- **Nhánh chính (Default Branch):** `main`
- **Nội dung bài nộp trên GitHub:** Repository đã được cấu hình `.gitignore` chuẩn (không chứa bất kỳ file nhị phân `ls`, `ls.exe` hay file `*.o`), đầy đủ mã nguồn modular, `Makefile` và báo cáo `README.md`.
