#include <stdio.h>
#include <string.h>
#include "fs.h"

int main() {
    int secim;
    char dosya_adi[32];
    char yeni_adi[32];
    char veri[1024];
    int offset, boyut;

    while (1) {
        printf("\nSimpleFS Menü:\n");
        printf("1. Format at\n");
        printf("2. Dosya oluştur\n");
        printf("3. Dosya sil\n");
        printf("4. Dosya listesi\n");
        printf("5. Dosyaya yaz\n");
        printf("6. Dosyadan oku\n");
        printf("7. Dosya ekle\n");
        printf("8. Dosya truncate (kes)\n");
        printf("9. Dosya kopyala\n");
        printf("10. Dosya yeniden adlandır\n");
        printf("11. Dosya boyutu\n");
        printf("12. Disk bütünlük kontrolü\n");
        printf("13. Disk birleştirme\n");
        printf("14. Disk yedekle\n");
        printf("15. Disk geri yükle\n");
        printf("16. Dosya içeriğini göster (cat)\n");
        printf("17. Dosyaları karşılaştır (diff)\n");
        printf("18. Dosya var mı?\n");
        printf("0. Çıkış\n");
        printf("Seçimin: ");
        scanf("%d", &secim);

        switch (secim) {
            case 1:
                fs_format();
                break;
            case 2:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                fs_create(dosya_adi);
                break;
            case 3:
                printf("Silinecek dosya: ");
                scanf("%s", dosya_adi);
                fs_delete(dosya_adi);
                break;
            case 4:
                fs_ls();
                break;
            case 5:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Veri girin: ");
                getchar(); // \n temizle
                fgets(veri, sizeof(veri), stdin);
                boyut = strlen(veri);
                if (veri[boyut - 1] == '\n') veri[--boyut] = '\0';
                fs_write(dosya_adi, veri, boyut);
                break;
            case 6:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Offset: ");
                scanf("%d", &offset);
                printf("Boyut: ");
                scanf("%d", &boyut);
                fs_read(dosya_adi, offset, boyut, veri);
                break;
            case 7:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Eklenecek veri: ");
                getchar(); // \n temizle
                fgets(veri, sizeof(veri), stdin);
                boyut = strlen(veri);
                if (veri[boyut - 1] == '\n') veri[--boyut] = '\0';
                fs_append(dosya_adi, veri, boyut);
                break;
            case 8:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Yeni boyut: ");
                scanf("%d", &boyut);
                fs_truncate(dosya_adi, boyut);
                break;
            case 9:
                printf("Kaynak dosya: ");
                scanf("%s", dosya_adi);
                printf("Hedef dosya: ");
                scanf("%s", yeni_adi);
                fs_copy(dosya_adi, yeni_adi);
                break;
            case 10:
                printf("Mevcut dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Yeni ad: ");
                scanf("%s", yeni_adi);
                fs_mv(dosya_adi, yeni_adi);
                break;
            case 11:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                fs_size(dosya_adi);
                break;
            case 12:
                fs_check_integrity();
                break;
            case 13:
                fs_defragment();
                break;
            case 14:
                printf("Yedek dosya adı: ");
                scanf("%s", yeni_adi);
                fs_backup(yeni_adi);
                break;
            case 15:
                printf("Yedek dosya adı: ");
                scanf("%s", yeni_adi);
                fs_restore(yeni_adi);
                break;
            case 16:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                fs_cat(dosya_adi);
                break;
            case 17:
                printf("Dosya 1: ");
                scanf("%s", dosya_adi);
                printf("Dosya 2: ");
                scanf("%s", yeni_adi);
                fs_diff(dosya_adi, yeni_adi);
                break;
            case 18:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                if (fs_exists(dosya_adi))
                    printf("Dosya mevcut.\n");
                else
                    printf("Dosya yok.\n");
                break;
            case 0:
                printf("Çıkılıyor...\n");
                return 0;
            default:
                printf("Geçersiz seçim!\n");
        }
    }

    return 0;
}

