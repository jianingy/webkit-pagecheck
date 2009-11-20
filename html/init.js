function loadScript(url)
{
   var e = document.createElement("script");
   e.src = url;
   e.type="text/javascript";
   document.getElementsByTagName("head")[0].appendChild(e);
}

loadScript("html/jquery-1.3.2.min.js");
loadScript("html/watch.js");

