using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Threading.Tasks;

namespace ApiWebBlog.Models
{
    public class TelegramClass
    {
        static string pathMessage = Directory.GetCurrentDirectory() + @"/wwwroot/content/Garaj/telegram";


        public static string TelegramSendMessage(string text, string apilToken, string destID)
        {
            var cDate = DateTime.Now.ToString("dd.MM.yyyy HH:mm");

            string sText = cDate + " - "+ text + " ("+SettingClass.name+")";

            string urlString = $"https://api.telegram.org/bot{apilToken}/sendMessage?chat_id={destID}&text={sText}";

            WebClient webclient = new WebClient();

            return webclient.DownloadString(urlString);
        }

        /* public static string TelegramGetMessage(string apilToken, string destID)
         {
             string urlString = $"https://api.telegram.org/bot{apilToken}/sendMessage?chat_id={destID}&text={text}";

             WebClient webclient = new WebClient();

             return webclient.DownloadString(urlString);
         }*/

        public static MessageTelegramClass TelegramGetMessage(string apilToken, string destID)
        {
            int ChatTimeout = 3;
            int UpdateId = -1;
            string rMessage;


            string[] allowedUpdates = { "message", "channel_post", "inline_query", "chosen_inline_result", "callback_query" };
            var jsonAllowedUpdates = JsonConvert.SerializeObject(allowedUpdates);

            string messageId = TelegramClass.ReadLastMessageId();

            if (messageId == null) return new MessageTelegramClass("error get last id");

            WebClient webclient = new WebClient();

            try
            {
                string urlString = $"https://api.telegram.org/bot{apilToken}/getUpdates?offset={UpdateId}&allowed_updates={jsonAllowedUpdates}&timeout={ChatTimeout}";

                rMessage = webclient.DownloadString(urlString);

            }
            catch
            {

                return new MessageTelegramClass("error get update");

            }

            dynamic msg=JsonConvert.DeserializeObject(rMessage);

            MessageTelegramClass message = new MessageTelegramClass
            {
                update_id = msg.result[0].update_id,
                message_id = msg.result[0].message.message_id,
                text = msg.result[0].message.text
            };

            if (Convert.ToInt32(message.message_id) <= Convert.ToInt32(messageId)) 
            {
                return new MessageTelegramClass("no message");
            }

            return message;

        }

        public static bool SaveDataTelegram(MessageTelegramClass message)
        {
             try
            {

                File.WriteAllText(pathMessage, message.message_id);

                return true;

            }
            catch
            {
                return false;
            }

        }


        public static string ReadLastMessageId()
        {

            try
            {

                return File.ReadAllText(pathMessage);

            }
            catch
            {
                return null;
            }

        }
    }
}
