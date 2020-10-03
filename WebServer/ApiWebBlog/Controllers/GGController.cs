using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using ApiWebBlog.Models;
using System.IO;

namespace ApiWebBlog.Controllers
{
    public class GGController : Controller
    {

        public IActionResult Index(int SCntV)
        {

            if (SCntV <= 0) { SCntV = 100; }
            ViewData["rmenu"] = 1;
            ViewData["MassDataTemp"] = GarageClass.GetDataForView(SCntV, out int cntOllLine);
            ViewData["cntOllLine"] = cntOllLine;
            ViewData["SCntV"] = SCntV;

            return View();
        }

        //сохранение температуры

        public String STG(string t1, string t2, string hum)
        {
            if (t1 != "" && t2 != "" && hum != "")
            {
                if (GarageClass.SaveDataTempGaraj(t1, t2, hum))
                {
                    return "YES";
                }
            }

            return "NO";
        }

        //сохранение сообщения
        public String SSG(string m)
        {
            if (GarageClass.SaveDataStatusGaraj(m))
            {
                return "YES";
            }

            return "NO";
        }


        public String GCMD(int status)
        {
            String[] cmds = { "r1on", "r2on", "r1off", "r2off", "r1togle", "r2togle", "roff","reboot" };
           
           var message= TelegramClass.TelegramGetMessage(SettingClass.TelegramApilToken,SettingClass.TelegramDestID);

            if (message.error == "")
            {
                if (TelegramClass.SaveDataTelegram(message))
                {
                    var cmd = message.text.Trim().Split(" ", StringSplitOptions.RemoveEmptyEntries);

                    if (cmd[0] == SettingClass.name) 
                    {
                        string text = "Команда " + cmd[1] + " прочитанна";

                        if (cmd[1]=="help") 
                        {
                            text = "";

                            foreach (var c in cmds) 
                            {
                                text += "\r\n" + c;
                            
                            }

                            text += "\r\n";
                        
                        }
                        

                        TelegramClass.TelegramSendMessage(text,SettingClass.TelegramApilToken, SettingClass.TelegramDestID);

                        return cmd[1];
                    }

                    return "noMessage";
                }
            }
            else if (message.error == "no message") return "noMessage";
            

                return "error";
        }
    }
}
