#include "telegram.h"
#include "misc.h"
#include "webget.h"
#include "ini_reader.h"
#include "logger.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <curl/curl.h>

// Global variables for Telegram configuration
bool telegram_enabled = false;
std::string telegram_bot_token;
std::string telegram_chat_id;
bool telegram_send_caption = true;

void loadTelegramConfig()
{
    INIReader ini;
    ini.ParseFile("pref.ini");
    telegram_enabled = ini.GetBool("telegram", "enable_telegram");
    telegram_bot_token = ini.Get("telegram", "telegram_bot_token");
    telegram_chat_id = ini.Get("telegram", "telegram_chat_id");
    telegram_send_caption = ini.GetBool("telegram", "telegram_send_caption");
}

bool isTelegramEnabled()
{
    loadTelegramConfig();
    return telegram_enabled && !telegram_bot_token.empty() && !telegram_chat_id.empty();
}

std::string getTelegramBotToken()
{
    loadTelegramConfig();
    return telegram_bot_token;
}

std::string getTelegramChatId()
{
    loadTelegramConfig();
    return telegram_chat_id;
}

bool getTelegramSendCaption()
{
    loadTelegramConfig();
    return telegram_send_caption;
}

std::string generateResultCaption(const std::vector<nodeInfo> &nodes, const std::string &group_name)
{
    if(nodes.empty())
        return "";
    
    std::stringstream caption;
    caption << "🚀 Stair Speedtest Results";
    
    if(!group_name.empty())
        caption << " - " << group_name;
    
    caption << "\n\n";
    
    // Count nodes by status
    int total_nodes = nodes.size();
    int working_nodes = 0;
    int failed_nodes = 0;
    double total_speed = 0.0;
    double max_speed = 0.0;
    double min_ping = 9999.0;
    
    for(const auto &node : nodes)
    {
        if(node.online)
        {
            working_nodes++;
            
            // Parse avgSpeed string (remove "MB/s", "KB/s", etc.)
            std::string speed_str = node.avgSpeed;
            if(speed_str != "N/A" && !speed_str.empty())
            {
                try 
                {
                    // Extract numeric value from speed string
                    size_t pos = speed_str.find_first_not_of("0123456789.");
                    if(pos != std::string::npos)
                        speed_str = speed_str.substr(0, pos);
                    double speed_val = std::stod(speed_str);
                    total_speed += speed_val;
                    if(speed_val > max_speed)
                        max_speed = speed_val;
                }
                catch(...) 
                {
                    // Ignore parsing errors
                }
            }
            
            // Parse avgPing string
            std::string ping_str = node.avgPing;
            if(ping_str != "0.00" && !ping_str.empty())
            {
                try 
                {
                    double ping_val = std::stod(ping_str);
                    if(ping_val < min_ping && ping_val > 0)
                        min_ping = ping_val;
                }
                catch(...)
                {
                    // Ignore parsing errors
                }
            }
        }
        else
        {
            failed_nodes++;
        }
    }
    
    caption << "📊 Summary:\n";
    caption << "• Total Nodes: " << total_nodes << "\n";
    caption << "• Working: " << working_nodes << " ✅\n";
    caption << "• Failed: " << failed_nodes << " ❌\n";
    
    if(working_nodes > 0)
    {
        caption << "• Avg Speed: " << std::fixed << std::setprecision(2) 
                << (total_speed / working_nodes) << " MB/s\n";
        caption << "• Max Speed: " << std::fixed << std::setprecision(2) 
                << max_speed << " MB/s\n";
        if(min_ping < 9999.0)
            caption << "• Best Ping: " << std::fixed << std::setprecision(2) << min_ping << "ms\n";
    }
    
    caption << "\n⏰ Test completed at: " << getCurrentTime();
    
    return caption.str();
}

// Callback function for curl to handle response data
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

bool sendTelegramPhoto(const std::string &bot_token, const std::string &chat_id, 
                      const std::string &photo_path, const std::string &caption)
{
    if(bot_token.empty() || chat_id.empty() || photo_path.empty())
    {
        writeLog(LOG_TYPE_ERROR, "Telegram: Missing required parameters");
        return false;
    }
    
    // Check if file exists
    std::ifstream file(photo_path, std::ios::binary);
    if(!file.good())
    {
        writeLog(LOG_TYPE_ERROR, "Telegram: Photo file not found: " + photo_path);
        return false;
    }
    file.close();
    
    CURL *curl;
    CURLcode res;
    std::string response;
    
    curl = curl_easy_init();
    if(!curl)
    {
        writeLog(LOG_TYPE_ERROR, "Telegram: Failed to initialize CURL");
        return false;
    }
    
    // Telegram API URL
    std::string url = "https://api.telegram.org/bot" + bot_token + "/sendPhoto";
    
    // Set up form data
    curl_mime *form = curl_mime_init(curl);
    curl_mimepart *field = nullptr;
    
    // Add chat_id field
    field = curl_mime_addpart(form);
    curl_mime_name(field, "chat_id");
    curl_mime_data(field, chat_id.c_str(), CURL_ZERO_TERMINATED);
    
    // Add photo field
    field = curl_mime_addpart(form);
    curl_mime_name(field, "photo");
    curl_mime_filedata(field, photo_path.c_str());
    
    // Add caption if provided
    if(!caption.empty())
    {
        field = curl_mime_addpart(form);
        curl_mime_name(field, "caption");
        curl_mime_data(field, caption.c_str(), CURL_ZERO_TERMINATED);
    }
    
    // Configure CURL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Perform the request
    writeLog(LOG_TYPE_INFO, "Telegram: Sending photo to chat " + chat_id);
    res = curl_easy_perform(curl);
    
    // Cleanup
    curl_mime_free(form);
    curl_easy_cleanup(curl);
    
    if(res != CURLE_OK)
    {
        writeLog(LOG_TYPE_ERROR, "Telegram: Failed to send photo: " + std::string(curl_easy_strerror(res)));
        return false;
    }
    
    // Check if response contains "ok":true
    if(response.find("\"ok\":true") != std::string::npos)
    {
        writeLog(LOG_TYPE_INFO, "Telegram: Photo sent successfully");
        return true;
    }
    else
    {
        writeLog(LOG_TYPE_ERROR, "Telegram: API error - " + response);
        return false;
    }
}