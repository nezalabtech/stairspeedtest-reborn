# Telegram Integration for Stair Speedtest

This enhanced version of Stair Speedtest includes automatic Telegram notification support. After each speed test completes and generates a PNG result image, the tool can automatically send the image to your Telegram chat.

## Setup

### 1. Create a Telegram Bot

1. Open Telegram and search for `@BotFather`
2. Start a chat with BotFather and send `/newbot`
3. Follow the instructions to create your bot
4. Save the **Bot Token** that BotFather provides (looks like `123456789:ABCdefGHIjklMNOpqrsTUVwxyz`)

### 2. Get Your Chat ID

1. Start a chat with your newly created bot
2. Send any message to your bot
3. Search for `@userinfobot` on Telegram and start a chat
4. Send any message to get your **Chat ID** (a number like `123456789`)

Alternatively, you can use this method:
1. Send a message to your bot
2. Visit: `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates`
3. Look for the `"chat":{"id":` value in the response

### 3. Configure Stair Speedtest

Edit the `pref.ini` file in the base directory and update the `[telegram]` section:

```ini
[telegram]
;Enable Telegram notification
enable_telegram=true

;Telegram Bot Token (get from @BotFather)
telegram_bot_token=123456789:ABCdefGHIjklMNOpqrsTUVwxyz

;Telegram Chat ID (use @userinfobot to get your chat ID)
telegram_chat_id=123456789

;Send image caption with test results
telegram_send_caption=true
```

## Features

### Automatic Image Sending
- Sends PNG result images automatically after test completion
- Works with single tests, multilink tests, and group tests
- Supports both web GUI and CLI modes

### Smart Captions
When `telegram_send_caption=true`, each image includes:
- 🚀 Test type (single/multilink/group name)
- 📊 Summary statistics:
  - Total nodes tested
  - Working vs failed nodes (✅❌)
  - Average and maximum speeds
  - Best ping time
- ⏰ Test completion timestamp

### Example Caption
```
🚀 Stair Speedtest Results - Group 1

📊 Summary:
• Total Nodes: 10
• Working: 8 ✅
• Failed: 2 ❌
• Avg Speed: 45.67 MB/s
• Max Speed: 89.12 MB/s
• Best Ping: 23.45ms

⏰ Test completed at: 2025-10-31 14:30:22
```

## Usage

Once configured, the Telegram feature works automatically:

1. Run any speed test (CLI or Web GUI)
2. Wait for the test to complete and image generation
3. Check your Telegram chat for the result image

### CLI Example
```bash
./stairspeedtest
# Enter your subscription URL when prompted
# After test completion, check Telegram for results
```

### Web GUI Example
```bash
./webserver.sh
# Open http://127.0.0.1:10870 in your browser
# Run tests through the web interface
# Results will be sent to Telegram automatically
```

## Troubleshooting

### Check Logs
The tool logs Telegram operations. Look for messages like:
- `Sending result to Telegram...`
- `Successfully sent result to Telegram`
- `Failed to send result to Telegram`

### Common Issues

1. **Bot Token Invalid**
   - Verify your bot token is correct
   - Make sure there are no extra spaces in `pref.ini`

2. **Chat ID Invalid**
   - Ensure you've sent at least one message to your bot
   - Verify the chat ID is a number (not username)

3. **Image Not Found**
   - Check if PNG files are being generated in `results/` folder
   - Ensure image generation is enabled in `pref.ini`

4. **Network Issues**
   - Verify internet connectivity
   - Check if Telegram API is accessible from your network

## Security Notes

- Keep your bot token secure and don't share it publicly
- The bot token in `pref.ini` gives access to your Telegram bot
- Consider using environment variables for sensitive data in production

## Integration with CI/CD

For automated testing environments, you can:
1. Set bot token and chat ID via environment variables
2. Modify the code to read from environment if `pref.ini` values are empty
3. Use different chat IDs for different environments (dev/staging/prod)