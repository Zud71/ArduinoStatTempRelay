using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Threading.Tasks;

namespace ApiWebBlog.Models
{
    public class SettingClass
    {
        public static string UseConnection { get; set; }
        public static string TelegramApilToken { get; set; }
        public static string TelegramDestID { get; set; }
        public static string name { get; set; }

    }

    public class MessageTelegramClass
    {
        public string update_id { get; set; }
        public string message_id { get; set; }
        public string text { get; set; }
        public string error { get; set; }

        public MessageTelegramClass(String mError="") 
        {
            error = mError;
        }
    }

}
