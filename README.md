# SistemProgramlamaProje
# SimpleFS - Basit Dosya Sistemi Simülatörü

SimpleFS, C dilinde yazılmış bir dosya sistemi simülatörüdür. Gerçek bir dosya sistemine yazım yapmadan, sistem çağrıları (`open`, `read`, `write`, `lseek`, `close`, `ftruncate` vb.) aracılığıyla sanal bir disk dosyası (Örn: `disk.sim`) üzerinde temel dosya işlemleri gerçekleştirir.

---

## 🌟 Proje Amacı

Bu projenin temel amacı, öğrencilerin sistem seviyesinde dosya işlemlerini öğrenmelerini sağlamak ve basit bir dosya sisteminin iç yapısını anlamalarını kolaylaştırmaktır.

**Kazandırılan Yetkinlikler:**

* Sistem çağrılarını doğru ve etkin kullanabilme
* Metadata yapıları ile dosya izleme
* Bellek blokları ve ofset hesaplama
* Gerçek dosya sistemine zarar vermeden deneme yapabilme

---

## ⚙️ Kurulum ve Derleme

### 🔍 Gereksinimler

* Linux tabanlı bir sistem (Ubuntu)
* `gcc` derleyicisi
* `make` komutu

Gerekli paketler yoksa yüklemek için:

```bash
sudo apt update
sudo apt install build-essential
```

### 📂 Derleme Adımları

1. Terminalden proje klasörüne gidin:

```bash
cd ~/Desktop/SimpleFS
```

2. Programı derlemek için:

```bash
make
```

3. Oluşan çalıştırılabilir dosyayı başlatmak için:

```bash
./simplefs
```

4. Temizlik yapmak için (opsiyonel):

```bash
make clean
```

> `make clean` komutu `simplefs`, `*.o`, `disk.sim` ve `log.txt` dosyalarını siler.

---

## 🔧 Program Özellikleri

SimpleFS, aşağıdaki dosya sistemi fonksiyonlarını destekler:

### ▶ï️ Temel Fonksiyonlar

| No | Komut         | Açıklama                                      |
| -- | ------------- | --------------------------------------------- |
| 1  | Format at     | `fs_format` - Sanal diski baştan oluşturur    |
| 2  | Dosya oluştur | `fs_create` - Yeni bir dosya ekler            |
| 3  | Dosya sil     | `fs_delete` - Dosyayı siler ve alanı temizler |
| 4  | Dosya listesi | `fs_ls` - Tüm dosyaları listeler              |

### ✍️ Veri işlemleri

| No | Komut          | Açıklama                                    |
| -- | -------------- | ------------------------------------------- |
| 5  | Dosyaya yaz    | `fs_write` - Dosyaya veri yazar             |
| 6  | Dosyadan oku   | `fs_read` - Belirli boyutta veri okur       |
| 7  | Veri ekle      | `fs_append` - Dosyanın sonuna veri ekler    |
| 8  | Truncate (kes) | `fs_truncate` - Dosyayı kısaltır/genişletir |

### 📁 Dosya Yönetimi

| No | Komut               | Açıklama                                                          |
| -- | ------------------- | ----------------------------------------------------------------- |
| 9  | Dosya kopyala       | `fs_copy` - Bir dosyayı yeni isimle kopyalar                      |
| 10 | Yeniden adlandır    | `fs_mv` - Dosya adını değiştirir                                  |
| 11 | Boyut görüntüle     | `fs_size` - Dosyanın boyutunu verir                               |
| 12 | Bütünlük kontrolü   | `fs_check_integrity` - Metadata ile veri tutarlığını kontrol eder |
| 13 | Disk birleştirme    | `fs_defragment` - Boş alanları birleştirir                        |
| 14 | Disk yedekle        | `fs_backup` - `disk.sim` dosyasını yedekler                       |
| 15 | Geri yükle          | `fs_restore` - Yedeği geri yükler                                 |
| 16 | Dosya içeriğini gör | `fs_cat` - Dosya içeriğini terminale yazdırır                     |
| 17 | Dosya karşılaştır   | `fs_diff` - İki dosya aynı mı kontrol eder                        |
| 18 | Dosya var mı?       | `fs_exists` - Belirtilen dosya mevcut mu kontrol eder             |
| 0  | Çıkış               | Programdan çıkar                                                  |

---

## 📆 Dosya Yapısı

```bash
SimpleFS/
├── main.c         # Kullanıcı menü interaktif sistemi
├── fs.c           # Dosya sistemi fonksiyonları
├── fs.h           # Yapı tanımları ve fonksiyon prototipleri
├── Makefile       # Derleme dosyası
├── disk.sim       # Sanal disk (runtime oluşur)
├── log.txt        # İşlem log dosyası (runtime oluşur)
```

---

## 📊 Teknik Detaylar

* Disk boyutu: **1 MB** (`1048576` byte)
* Metadata alanı: ilk `4 KB`
* Blok boyutu: sabit (`1 KB`)
* Maksimum dosya sayısı: `64`
* Her dosya için tutulur: isim, boyut, başlangıç bloğu, oluşturulma tarihi, aktiflik durumu

---

## 📄 Log Kayıtları

Tüm dosya sistemine yapılan işlemler `log.txt` dosyasında zaman damgası ile tutulur.

Örnek:

```
[Mon May 20 13:45:12 2025] Dosya yazıldı: notlar.txt (58 byte)
```

---

## 📈 Test Senaryosu Örneği

1. Format at
2. "veri.txt" dosyasını oluştur
3. "merhaba dunya" yaz
4. İçeriği oku
5. Boyutunu görüntüle
6. Kopyala: "yedek.txt"
7. Karşılaştır: `fs_diff(veri.txt, yedek.txt)`
8. "veri.txt" dosyasını sil
9. Defragment yap

---

## 👩‍💻 Geliştiriciler

* Öğrenci 1: G231210372 - Sila Canga
* Öğrenci 2: G231210370 - Aleyna Cakir

---

## 📖 Kaynaklar

* [man7.org - Linux System Calls](https://man7.org/linux/man-pages/)
* [OSTEP - Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/)
* [Linux Programmer's Manual](https://man7.org/linux/man-pages/dir_section_2.html)

---

> ✨ Bu proje 2024-2025 BM 308 Sistem Programlama dersi kapsaımında gerçekleştirilmiştir.

