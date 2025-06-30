#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "fs.h"


int ftruncate(int fd, off_t length);


void fs_format() {
    int fd = open("disk.sim", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    ftruncate(fd, 1048576);  // 1MB

    Metadata metadata;
    memset(&metadata, 0, sizeof(Metadata));

    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));

    printf("Dosya sistemi başarıyla formatlandı.\n");
    close(fd);
}

void fs_create(const char* filename) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }
    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            printf("HATA: '%s' isminde bir dosya zaten var.\n", filename);
            close(fd);
            return;
        }
    }

    int index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!metadata.files[i].is_used) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("HATA: Maksimum dosya sayısına ulaşıldı.\n");
        close(fd);
        return;
    }

    off_t next_start = METADATA_SIZE;
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used) {
            next_start = metadata.files[i].start_block + metadata.files[i].size;
        }
    }

    FileEntry* new_file = &metadata.files[index];
    strncpy(new_file->name, filename, MAX_FILENAME);
    new_file->size = 0;
    new_file->created_at = time(NULL);
    new_file->is_used = 1;
    new_file->start_block = next_start;

    metadata.file_count++;

    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));

    printf("Dosya '%s' başarıyla oluşturuldu.\n", filename);
    close(fd);
}

void fs_ls() {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) {
        perror("disk.sim açılamadı");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    printf("\nDiskteki Dosyalar:\n");
    printf("--------------------\n");

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used) {
            char* tarih = ctime(&metadata.files[i].created_at);
            tarih[strlen(tarih) - 1] = '\0';
            printf("📄 %s\t| %d byte\t| Oluşturma: %s\n", metadata.files[i].name, metadata.files[i].size, tarih);
        }
    }

    close(fd);
}

void fs_write(const char* filename, const char* data, int size) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            off_t start = metadata.files[i].start_block;
            int old_size = metadata.files[i].size;

            // Disk boyut kontrolü
            if (start + size > 1048576) {
                printf("HATA: Diskte yeterli alan yok.\n");
                close(fd);
                return;
            }

            // Yeni veriyi yaz
            lseek(fd, start, SEEK_SET);
            ssize_t written = write(fd, data, size);
            if (written != size) {
                printf("HATA: Veri tam olarak yazılamadı!\n");
                close(fd);
                return;
            }

            // Önceki veriden daha kısaysa kalan kısmı sıfırla
            if (size < old_size) {
                int diff = old_size - size;
                char* zeros = calloc(1, diff);
                if (zeros) {
                    write(fd, zeros, diff);
                    free(zeros);
                }
            }

            // Metadata güncelle
            metadata.files[i].size = size;
            lseek(fd, 0, SEEK_SET);
            write(fd, &metadata, sizeof(Metadata));

            printf("Dosya '%s' içine %d byte veri yazıldı.\n", filename, size);

            // Logla
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Dosyaya yazıldı: %s (%d byte)", filename, size);
            append_to_log(log_msg);

            close(fd);
            return;
        }
    }

    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_read(const char* filename, int offset, int size, char* buffer) {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            if (offset < 0 || offset + size > metadata.files[i].size) {
                printf("HATA: Okuma boyutu dosya boyutunu aşıyor.\n");
                close(fd);
                return;
            }

            lseek(fd, metadata.files[i].start_block + offset, SEEK_SET);
            ssize_t okunan = read(fd, buffer, size);
            if (okunan <= 0) {
                printf("HATA: Veri okunamadı!\n");
                close(fd);
                return;
            }

            // buffer[okunan] = '\0'; // KALDIRILDI!
            // Veriyi yazdırmak istiyorsan fs_cat gibi yerlerde işlem yap
            printf("Dosya '%s' içeriği (ilk %ld byte):\n", filename, okunan);
            write(STDOUT_FILENO, buffer, okunan);
            printf("\n");

            close(fd);
            return;
        }
    }

    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_delete(const char* filename) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            // Veri alanını sıfırla
            char* zero_buf = calloc(1, metadata.files[i].size);
            if (zero_buf) {
                lseek(fd, metadata.files[i].start_block, SEEK_SET);
                write(fd, zero_buf, metadata.files[i].size);
                free(zero_buf);
            }

            // Metadata temizliği
            metadata.files[i].is_used = 0;
            metadata.files[i].size = 0;
            metadata.files[i].name[0] = '\0';
            metadata.files[i].start_block = 0;
            metadata.files[i].created_at = 0;
            metadata.file_count--;

            // Metadata diske yaz
            lseek(fd, 0, SEEK_SET);
            write(fd, &metadata, sizeof(Metadata));

            printf("Dosya '%s' silindi ve disk alanı sıfırlandı.\n", filename);

            // Log yaz
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Dosya silindi: %s", filename);
            append_to_log(log_msg);

            close(fd);
            return;
        }
    }

    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_append(const char* filename, const char* data, int size) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            int current_size = metadata.files[i].size;
            off_t write_pos = metadata.files[i].start_block + current_size;

            // Disk sınırı kontrolü
            if (write_pos + size > 1048576) {
                printf("HATA: Diskte yeterli alan yok.\n");
                close(fd);
                return;
            }

            // Veriyi ekle
            lseek(fd, write_pos, SEEK_SET);
            write(fd, data, size);

            // Metadata güncelle
            metadata.files[i].size += size;
            lseek(fd, 0, SEEK_SET);
            write(fd, &metadata, sizeof(Metadata));

            printf("Dosya '%s' içine %d byte eklendi.\n", filename, size);

            // Log
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Dosyaya eklendi: %s (+%d byte)", filename, size);
            append_to_log(log_msg);

            close(fd);
            return;
        }
    }

    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_truncate(const char* filename, int new_size) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            int old_size = metadata.files[i].size;
            off_t start = metadata.files[i].start_block;

            if (new_size < old_size) {
                metadata.files[i].size = new_size;
                printf("Dosya '%s' %d byte'a kesildi.\n", filename, new_size);
            } else if (new_size > old_size) {
                // Disk sınırı kontrolü
                if (start + new_size > 1048576) {
                    printf("HATA: Diskte yeterli alan yok.\n");
                    close(fd);
                    return;
                }

                // Yeni alanı sıfırla
                int diff = new_size - old_size;
                char* zeros = calloc(1, diff);
                if (zeros) {
                    lseek(fd, start + old_size, SEEK_SET);
                    write(fd, zeros, diff);
                    free(zeros);
                    metadata.files[i].size = new_size;
                    printf("Dosya '%s' %d byte'a genişletildi.\n", filename, new_size);
                } else {
                    printf("HATA: Bellek ayrılamadı.\n");
                }
            } else {
                printf("Dosya zaten %d byte.\n", new_size);
            }

            // Metadata güncelle
            lseek(fd, 0, SEEK_SET);
            write(fd, &metadata, sizeof(Metadata));

            // Log
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Dosya truncate edildi: %s -> %d byte", filename, new_size);
            append_to_log(log_msg);

            close(fd);
            return;
        }
    }

    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_copy(const char* src, const char* dest) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) return;

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    int src_index = -1, dest_index = -1;
    off_t next_start = METADATA_SIZE;

    // En son biten dosyanın sonunu bul
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used) {
            off_t end = metadata.files[i].start_block + metadata.files[i].size;
            if (end > next_start)
                next_start = end;
        }
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, src) == 0)
            src_index = i;
        if (!metadata.files[i].is_used && dest_index == -1)
            dest_index = i;
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, dest) == 0) {
            printf("HATA: '%s' isminde zaten bir dosya var.\n", dest);
            close(fd);
            return;
        }
    }

    if (src_index == -1 || dest_index == -1) {
        printf("HATA: Kaynak dosya yok ya da hedef için yer yok.\n");
        close(fd);
        return;
    }

    FileEntry* src_file = &metadata.files[src_index];
    FileEntry* dest_file = &metadata.files[dest_index];

    // Kopyalama
    char* buffer = malloc(src_file->size);
    lseek(fd, src_file->start_block, SEEK_SET);
    read(fd, buffer, src_file->size);

    lseek(fd, next_start, SEEK_SET);
    write(fd, buffer, src_file->size);

    strncpy(dest_file->name, dest, MAX_FILENAME);
    dest_file->is_used = 1;
    dest_file->size = src_file->size;
    dest_file->start_block = next_start;
    dest_file->created_at = time(NULL);

    metadata.file_count++;

    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));

    free(buffer);

    printf("'%s' dosyası '%s' olarak kopyalandı.\n", src, dest);
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Dosya kopyalandı: %s -> %s", src, dest);
    append_to_log(log_msg);
    close(fd);
}


void fs_cat(const char* filename) {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            int size = metadata.files[i].size;
            if (size == 0) {
                printf("Dosya boş.\n");
                close(fd);
                return;
            }

            if (size > 4096) size = 4096;  // Maksimum 4KB göster

            char* buffer = malloc(size + 1);  // null terminator için +1
            if (!buffer) {
                perror("Bellek ayrımı başarısız");
                close(fd);
                return;
            }

            lseek(fd, metadata.files[i].start_block, SEEK_SET);
            read(fd, buffer, size);
            buffer[size] = '\0';  // Yazdırılabilir güvenli string

            printf("Dosya içeriği (%d byte):\n%s\n", size, buffer);

            free(buffer);
            close(fd);
            return;
        }
    }

    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_backup(const char* backup_filename) {
    int src_fd = open("disk.sim", O_RDONLY);
    int dst_fd = open(backup_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (src_fd < 0 || dst_fd < 0) {
        perror("Yedekleme hatası");
        return;
    }
    char buffer[1024];
    ssize_t n;
    while ((n = read(src_fd, buffer, sizeof(buffer))) > 0) {
        write(dst_fd, buffer, n);
    }
    printf("Yedekleme tamamlandı: %s\n", backup_filename);
    close(src_fd);
    close(dst_fd);
}

void fs_restore(const char* backup_filename) {
    int src_fd = open(backup_filename, O_RDONLY);
    int dst_fd = open("disk.sim", O_WRONLY | O_TRUNC);
    if (src_fd < 0 || dst_fd < 0) {
        perror("Geri yükleme hatası");
        return;
    }
    char buffer[1024];
    ssize_t n;
    while ((n = read(src_fd, buffer, sizeof(buffer))) > 0) {
        write(dst_fd, buffer, n);
    }
    printf("Geri yükleme tamamlandı.\n");
    close(src_fd);
    close(dst_fd);
}

void fs_diff(const char* f1, const char* f2) {
    char buf1[4096], buf2[4096];
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) return;

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    FileEntry *file1 = NULL, *file2 = NULL;
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, f1) == 0)
            file1 = &metadata.files[i];
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, f2) == 0)
            file2 = &metadata.files[i];
    }

    if (!file1 || !file2) {
        printf("HATA: Dosyalardan biri bulunamadı.\n");
        close(fd);
        return;
    }

    if (file1->size != file2->size) {
        printf("Dosyalar farklı (farklı boyut).\n");
        close(fd);
        return;
    }

    lseek(fd, file1->start_block, SEEK_SET);
    read(fd, buf1, file1->size);
    lseek(fd, file2->start_block, SEEK_SET);
    read(fd, buf2, file2->size);

    if (memcmp(buf1, buf2, file1->size) == 0)
        printf("Dosyalar aynı.\n");
    else
        printf("Dosyalar farklı (içerik).\n");

    close(fd);
}

void fs_log() {
    FILE* log = fopen("log.txt", "r");
    if (!log) {
        printf("Log dosyası yok veya okunamıyor.\n");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), log)) {
        printf("%s", line);
    }
    fclose(log);
}

void append_to_log(const char* message) {
    FILE* log = fopen("log.txt", "a");
    if (log) {
        time_t now = time(NULL);
        char* time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0'; // \n temizle
        fprintf(log, "[%s] %s\n", time_str, message);
        fclose(log);
    }
}

void fs_mv(const char* oldname, const char* newname) {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    int old_index = -1;

    // Aynı ada sahip başka dosya var mı? Kontrol et
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used) {
            if (strcmp(metadata.files[i].name, newname) == 0) {
                printf("HATA: '%s' isminde zaten bir dosya var.\n", newname);
                close(fd);
                return;
            }

            if (strcmp(metadata.files[i].name, oldname) == 0) {
                old_index = i;
            }
        }
    }

    if (old_index == -1) {
        printf("HATA: '%s' isimli dosya bulunamadı.\n", oldname);
        close(fd);
        return;
    }

    // Yeniden adlandır
    strncpy(metadata.files[old_index].name, newname, MAX_FILENAME);

    // Metadata güncelle
    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));

    printf("Dosya '%s' -> '%s' olarak yeniden adlandırıldı.\n", oldname, newname);

    // Log
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Dosya adı değiştirildi: %s -> %s", oldname, newname);
    append_to_log(log_msg);

    close(fd);
}


void fs_size(const char* filename) {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) return;
    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            printf("Dosya '%s' boyutu: %d byte\n", filename, metadata.files[i].size);
            close(fd);
            return;
        }
    }
    printf("HATA: Dosya '%s' bulunamadı.\n", filename);
    close(fd);
}

void fs_check_integrity() {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) return;
    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strlen(metadata.files[i].name) == 0) {
            printf("Uyarı: Kullanımda işaretli ancak isimsiz dosya bulundu.\n");
        }
    }
    printf("Disk bütünlüğü kontrol edildi.\n");
    append_to_log("Disk bütünlüğü kontrol edildi.");
    close(fd);
}

void fs_defragment() {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));

    // Yeni veri konumu başlangıcı
    off_t next_free = METADATA_SIZE;

    for (int i = 0; i < MAX_FILES; i++) {
        if (!metadata.files[i].is_used)
            continue;

        FileEntry* file = &metadata.files[i];

        if (file->start_block != next_free) {
            // Taşınması gerekiyor
            char* buffer = malloc(file->size);
            lseek(fd, file->start_block, SEEK_SET);
            read(fd, buffer, file->size);

            lseek(fd, next_free, SEEK_SET);
            write(fd, buffer, file->size);

            file->start_block = next_free;

            free(buffer);
        }

        next_free = file->start_block + file->size;
    }

    // Metadata güncelle
    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));

    printf("Disk başarıyla birleştirildi.\n");
    append_to_log("Disk birleştirildi.");
    close(fd);
}

int fs_exists(const char* filename) {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) {
        perror("disk.sim açılamadı (önce format atınız?)");
        return 0;
    }

    Metadata metadata;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));
    close(fd);

    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata.files[i].is_used && strcmp(metadata.files[i].name, filename) == 0) {
            return 1;
        }
    }

    return 0;
}

