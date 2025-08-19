#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
//#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_mac.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include <libssh2.h>
#include <libssh2_sftp.h>
#include "uto_eth_init.h"

static const char *TAG = "sftp";

// SFTP 서버 설정
#define SFTP_SERVER_IP "192.168.0.2"    // SFTP 서버 IP 주소
#define SFTP_SERVER_PORT 14022
#define SFTP_USERNAME "sftpuser2021"           // SFTP 사용자명
#define SFTP_PASSWORD "sftp@user2021"          // SFTP 비밀번호

void list_spiffs_files(const char *path);

// SFTP 연결 구조체
typedef struct {
    int socket;
    LIBSSH2_SESSION *session;
    LIBSSH2_SFTP *sftp_session;
    bool connected;
} sftp_connection_t;

// SFTP 연결 생성
static sftp_connection_t* sftp_connect(const char* server_ip, int port, const char* username, const char* password)
{
    sftp_connection_t* conn = malloc(sizeof(sftp_connection_t));
    if (!conn) {
        ESP_LOGE(TAG, "Failed to allocate connection structure");
        return NULL;
    }
    
    memset(conn, 0, sizeof(sftp_connection_t));
    
    // 소켓 생성
    conn->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (conn->socket == -1) {
        ESP_LOGE(TAG, "Failed to create socket");
        free(conn);
        return NULL;
    }
    
    // 서버 연결
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &sin.sin_addr);
    
    if (connect(conn->socket, (struct sockaddr*)&sin, sizeof(sin)) != 0) {
        ESP_LOGE(TAG, "Failed to connect to server");
        close(conn->socket);
        free(conn);
        return NULL;
    }
    
    // libssh2 초기화
    if (libssh2_init(0) != 0) {
        ESP_LOGE(TAG, "libssh2 initialization failed");
        close(conn->socket);
        free(conn);
        return NULL;
    }
    
    // SSH 세션 생성
    conn->session = libssh2_session_init();
    if (!conn->session) {
        ESP_LOGE(TAG, "Failed to create SSH session");
        close(conn->socket);
        libssh2_exit();
        free(conn);
        return NULL;
    }
    
    // SSH 핸드셰이크
    if (libssh2_session_handshake(conn->session, conn->socket) != 0) {
        ESP_LOGE(TAG, "SSH handshake failed");
        libssh2_session_free(conn->session);
        close(conn->socket);
        libssh2_exit();
        free(conn);
        return NULL;
    }
    
    // 인증
    if (libssh2_userauth_password(conn->session, username, password) != 0) {
        ESP_LOGE(TAG, "Authentication failed");
        libssh2_session_free(conn->session);
        close(conn->socket);
        libssh2_exit();
        free(conn);
        return NULL;
    }
    
    // SFTP 세션 초기화
    conn->sftp_session = libssh2_sftp_init(conn->session);
    if (!conn->sftp_session) {
        ESP_LOGE(TAG, "Failed to initialize SFTP session");
        libssh2_session_free(conn->session);
        close(conn->socket);
        libssh2_exit();
        free(conn);
        return NULL;
    }
    
    conn->connected = true;
    ESP_LOGI(TAG, "SFTP connection established successfully");
    return conn;
}

// SFTP 연결 종료
static void sftp_disconnect(sftp_connection_t* conn)
{
    if (!conn) return;
    
    if (conn->sftp_session) {
        libssh2_sftp_shutdown(conn->sftp_session);
    }
    
    if (conn->session) {
        libssh2_session_disconnect(conn->session, "Normal Shutdown");
        libssh2_session_free(conn->session);
    }
    
    if (conn->socket > 0) {
        close(conn->socket);
    }
    
    libssh2_exit();
    conn->connected = false;
    free(conn);
    
    ESP_LOGI(TAG, "SFTP connection closed");
}

// 스트리밍 파일 업로드 (메모리 효율적)
static int sftp_upload_file_streaming(sftp_connection_t* conn, 
                                      const char* local_file_path,
                                      const char* remote_file_path)
{
    if (!conn || !conn->connected) {
        ESP_LOGE(TAG, "Invalid connection");
        return -1;
    }
    
    // 로컬 파일 열기
    FILE* local_file = fopen(local_file_path, "rb");
    if (!local_file) {
        ESP_LOGE(TAG, "Failed to open local file: %s", local_file_path);
        return -1;
    }
    
    // 파일 크기 확인
    fseek(local_file, 0, SEEK_END);
    long file_size = ftell(local_file);
    fseek(local_file, 0, SEEK_SET);
    
    ESP_LOGI(TAG, "Uploading file: %s (%ld bytes)", local_file_path, file_size);
    
    // 원격 파일 열기
    LIBSSH2_SFTP_HANDLE* remote_file = libssh2_sftp_open(
        conn->sftp_session, remote_file_path,
        LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC,
        LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH
    );
    
    if (!remote_file) {
        ESP_LOGE(TAG, "Failed to open remote file: %s (error: %ld)", 
                remote_file_path, libssh2_sftp_last_error(conn->sftp_session));
        fclose(local_file);
        return -1;
    }
    
    // 청크 단위로 전송 (메모리 효율적)
    #define CHUNK_SIZE 4096
    char buffer[CHUNK_SIZE];
    size_t total_sent = 0;
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, local_file)) > 0) {
        size_t bytes_written = 0;
        
        while (bytes_written < bytes_read) {
            ssize_t result = libssh2_sftp_write(remote_file, 
                                              buffer + bytes_written, 
                                              bytes_read - bytes_written);
            if (result < 0) {
                ESP_LOGE(TAG, "Failed to write to remote file");
                goto cleanup;
            }
            bytes_written += result;
        }
        
        total_sent += bytes_read;
        
        // 진행률 표시
        if (file_size > 0) {
            int progress = (total_sent * 100) / file_size;
            ESP_LOGI(TAG, "Upload progress: %d%% (%d/%ld bytes)", 
                    progress, (int)total_sent, file_size);
        }
        
        // 다른 태스크에게 CPU 양보
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    ESP_LOGI(TAG, "Upload completed: %d bytes", (int)total_sent);
    
cleanup:
    libssh2_sftp_close(remote_file);
    fclose(local_file);
    
    return (total_sent == file_size) ? 0 : -1;
}

// 스트리밍 파일 다운로드
static int sftp_download_file_streaming(sftp_connection_t* conn,
                                       const char* remote_file_path,
                                       const char* local_file_path)
{
    if (!conn || !conn->connected) {
        ESP_LOGE(TAG, "Invalid connection");
        return -1;
    }
    
    // 원격 파일 열기
    LIBSSH2_SFTP_HANDLE* remote_file = libssh2_sftp_open(
        conn->sftp_session, remote_file_path, LIBSSH2_FXF_READ, 0
    );
    
    if (!remote_file) {
        ESP_LOGE(TAG, "Failed to open remote file: %s", remote_file_path);
        return -1;
    }
    
    // 로컬 파일 생성
    FILE* local_file = fopen(local_file_path, "wb");
    if (!local_file) {
        ESP_LOGE(TAG, "Failed to create local file: %s", local_file_path);
        libssh2_sftp_close(remote_file);
        return -1;
    }
    
    ESP_LOGI(TAG, "Downloading: %s -> %s", remote_file_path, local_file_path);
    
    // 청크 단위로 다운로드
    char buffer[CHUNK_SIZE];
    size_t total_received = 0;
    ssize_t bytes_read;
    
    while ((bytes_read = libssh2_sftp_read(remote_file, buffer, CHUNK_SIZE)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, local_file);
        if (bytes_written != bytes_read) {
            ESP_LOGE(TAG, "Failed to write to local file");
            break;
        }
        
        total_received += bytes_read;
        ESP_LOGI(TAG, "Downloaded: %d bytes", (int)total_received);
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    fclose(local_file);
    libssh2_sftp_close(remote_file);
    
    ESP_LOGI(TAG, "Download completed: %d bytes", (int)total_received);
    return 0;
}

// 여러 파일 일괄 처리 (세션 재사용)
#if 0
static void sftp_batch_operations(sftp_connection_t* conn)
{
    if (!conn || !conn->connected) {
        ESP_LOGE(TAG, "Invalid connection for batch operations");
        return;
    }
    
    // 여러 파일 업로드 (같은 세션 사용)
    const char* files_to_upload[] = {
        "/spiffs/file1.txt",
        "/spiffs/file2.txt", 
        "/spiffs/config.json"
    };
    
    const char* remote_names[] = {
        "esp32_file1.txt",
        "esp32_file2.txt",
        "esp32_config.json"
    };
    
    int num_files = sizeof(files_to_upload) / sizeof(files_to_upload[0]);
    
    for (int i = 0; i < num_files; i++) {
        ESP_LOGI(TAG, "Uploading file %d/%d: %s", i+1, num_files, files_to_upload[i]);
        
        if (sftp_upload_file_streaming(conn, files_to_upload[i], remote_names[i]) == 0) {
            ESP_LOGI(TAG, "✓ Upload successful: %s", files_to_upload[i]);
        } else {
            ESP_LOGE(TAG, "✗ Upload failed: %s", files_to_upload[i]);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500)); // 잠시 대기
    }
}
#endif

static void sftp_client_task(void *pvParameters)
{
    sftp_connection_t* conn;

	ESP_LOGI(TAG, "Starting improved SFTP client...");
    
    //--- 단일 파일 업로드 테스트 ----------------------------------------------------------------------
	if( (conn= sftp_connect( SFTP_SERVER_IP, SFTP_SERVER_PORT, SFTP_USERNAME, SFTP_PASSWORD ))==NULL ) {
        ESP_LOGE(TAG, "Failed to establish SFTP connection");
        vTaskDelete(NULL);
        return;
    }

    if( sftp_upload_file_streaming(conn, "/spiffs/upload.txt", "esp32_upload.txt")==0 ) {
        ESP_LOGI(TAG, "✓ Single file upload successful");
        
    }
    sftp_disconnect(conn); 	// 연결 종료

    vTaskDelay(pdMS_TO_TICKS(5000));	// wait 5 sec

	//--- 단일 파일 다운로드 테스트 -------------------------------------------------------------------
	if( (conn= sftp_connect( SFTP_SERVER_IP, SFTP_SERVER_PORT, SFTP_USERNAME, SFTP_PASSWORD ))==NULL ) {
        ESP_LOGE(TAG, "Failed to establish SFTP connection");
        vTaskDelete(NULL);
        return;
    }

	if (sftp_download_file_streaming(conn, "esp32_upload.txt", "/spiffs/downloaded.txt") == 0) {
		ESP_LOGI(TAG, "✓ File download successful");
	}
    sftp_disconnect(conn); 	// 연결 종료

  	list_spiffs_files("/spiffs");

    vTaskDelete(NULL);
}

void init_sftp_client_task(void)
{
    // 이더넷 연결 대기
    ESP_LOGI(TAG, "Waiting for ethernet connection...");
//  xEventGroupWaitBits(s_eth_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    while (!is_eth_connected()) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Ethernet connected, starting SFTP operations");

    // SFTP 클라이언트 태스크 생성
    xTaskCreate(sftp_client_task, "sftp_client_task", 8192, NULL, 5, NULL);
}