using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using ApiWebBlog.Models;
using Newtonsoft.Json.Linq;

namespace ApiWebBlog.Controllers
{
    public class LRController : Controller
    {
        public IActionResult Index()
        {
            return View();
        }


        [HttpPost]
        public JsonResult RigRegister([FromBody] JObject data)
        {



            return null;
        }

        [HttpPost]
        public JsonResult RigPing([FromBody] JObject data)
        {



            return null;
        }

    }
}
