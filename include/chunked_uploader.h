#ifndef CHUNKED_UPLOADER_H
#define CHUNKED_UPLOADER_H

#include <WiFiClient.h>
#include <HTTPClient.h>
#include <esp_camera.h>
#include <esp_task_wdt.h>

#define CHUNK_SIZE 8192        // 8 КБ — согласовано с сервером
#define MAX_RETRIES 3          // попыток на чанк
#define RETRY_DELAY_MS 1000    // задержка между попытками
#define CONNECT_TIMEOUT_MS 10000
#define DEBUG_MEMORY 1         // включить детальное логирование памяти

/**
 * @brief Класс для chunked-загрузки изображений с поддержкой resume
 * 
 * Поддерживает:
 * - Разбиение файла на чанки по 8 КБ
 * - Автоматический retry при сбоях
 * - Resume после разрыва соединения
 * - Логирование использования памяти (PSRAM + SRAM)
 */
class ChunkedUploader {
private:
    String serverHost;
    uint16_t serverPort;
    String camId;
    
    // Состояние upload
    String uploadId;
    size_t totalSize;
    size_t bytesUploaded;
    
    // Статистика памяти
    void logMemoryStats(const char* label);
    
    // Retry-механизм для одного чанка
    bool uploadChunkWithRetry(const uint8_t* data, size_t len, size_t offset);
    
    // Запрос статуса для resume
    bool getUploadStatus(size_t& resumeOffset);
    
    // Парсинг JSON без библиотеки
    String extractJsonValue(const String& json, const String& key);
    int extractJsonInt(const String& json, const String& key);
    
public:
    /**
     * @brief Конструктор
     * @param host Адрес сервера (IP или домен)
     * @param port Порт сервера
     * @param id ID камеры
     */
    ChunkedUploader(const String& host, uint16_t port, const String& id);
    
    /**
     * @brief Инициализация upload-сессии
     * @param fileSize Размер файла в байтах
     * @param fileHash MD5-хеш (опционально)
     * @return true при успехе
     */
    bool initUpload(size_t fileSize, const String& fileHash = "");
    
    /**
     * @brief Основная функция upload framebuffer с автоматическим resume
     * @param fb Указатель на framebuffer от esp_camera_fb_get()
     * @return true при успехе
     */
    bool uploadFramebuffer(camera_fb_t* fb);
    
    /**
     * @brief Финализация upload (проверка целостности на сервере)
     * @return true при успехе
     */
    bool finalize();
    
    /**
     * @brief Получение прогресса в процентах
     * @return Прогресс 0-100%
     */
    float getProgress() const;
    
    /**
     * @brief Получение текущего uploadId (для восстановления после reboot)
     * @return uploadId
     */
    String getUploadId() const { return uploadId; }
};

#endif // CHUNKED_UPLOADER_H