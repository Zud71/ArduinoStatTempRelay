using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace ApiWebBlog.Models
{
    public class GarageClass
    {
        public static string FullPath_wwwroot = Directory.GetCurrentDirectory() + @"/wwwroot/";

        public static bool SaveDataTempGaraj(string tmp1, string tmp2, string hum)
        {
            string fullpath = FullPath_wwwroot + "content/Garaj/stats";
           
            try
            {
                var dt = DateTime.Now.ToString("dd.MM.yyyy HH:mm:ss");
               
                File.AppendAllText(fullpath, dt + ";" + tmp1 + ";" + tmp2 + ";" + hum + ";\r\n");

                return true;

            }
            catch
            {
                return false;
            }

        }

        public static bool SaveDataStatusGaraj(string message)
        {
            string fullpath = FullPath_wwwroot + "content/Garaj/mesage";
            
            try
            {
                var dt = DateTime.Now.ToString("dd.MM.yyyy HH:mm:ss");
                File.AppendAllText(fullpath, dt + ";" + message + "\r\n");

                return true;

            }
            catch
            {
                return false;
            }

        }

       public struct DataForGaraj
        {
            public string DataTime;
            public string Value;
        }

        public static List<object> GetDataForView(int SCntV,out int cntOllLine)
        {
            string fullpath = FullPath_wwwroot + "content/Garaj/stats";
            

            List<object> Result = new List<object>();

            List<DataForGaraj> TempNiz=new List<DataForGaraj>();
            List<DataForGaraj> TempVerh = new List<DataForGaraj>();
            List<DataForGaraj> Vlaja = new List<DataForGaraj>();

            try
            {
                var rdFile = File.ReadAllLines(fullpath);
                cntOllLine = rdFile.Length;
                int cnt = 1;
                if (cntOllLine > SCntV)
                    cnt = cntOllLine / SCntV;

           

                for (int i = 0; i < rdFile.Length; i += cnt) 
                {
                    var splitData = rdFile[i].Split(';');

                    if (String.IsNullOrWhiteSpace(splitData[1]) || String.IsNullOrWhiteSpace(splitData[2]) || String.IsNullOrWhiteSpace(splitData[3])) 
                    {
                        continue;
                    }

                    TempNiz.Add(new DataForGaraj() { DataTime = splitData[0], Value = splitData[1] });

                    TempVerh.Add(new DataForGaraj() { DataTime = splitData[0], Value = splitData[2] });

                    Vlaja.Add(new DataForGaraj() { DataTime = splitData[0], Value = splitData[3] });
                }

              /*  foreach (string line in File.ReadLines(fullpath))
                {

                    var splitData=line.Split(';');

                    TempNiz.Add(new DataForGaraj() { DataTime = splitData[0], Value = splitData[1] });

                    TempVerh.Add(new DataForGaraj() { DataTime = splitData[0], Value = splitData[2] });

                    Vlaja.Add(new DataForGaraj() { DataTime = splitData[0], Value = splitData[3] });

                }*/

                Result.Add(TempNiz);
                Result.Add(TempVerh);
                Result.Add(Vlaja);

                return Result;


            }
            catch
            {
                cntOllLine = 0;
                return null;
            }

        }

    }
}
