function dateFormat(fmt, date) {
    let ret;
    let opt = {
        "Y+": date.getFullYear().toString(),        // 年
        "m+": (date.getMonth() + 1).toString(),     // 月
        "d+": date.getDate().toString(),            // 日
        "H+": date.getHours().toString(),           // 时
        "M+": date.getMinutes().toString(),         // 分
        "S+": date.getSeconds().toString()          // 秒
        // 有其他格式化字符需求可以继续添加，必须转化成字符串
    };
    for (let k in opt) {
        ret = new RegExp("(" + k + ")").exec(fmt);
        if (ret) {
            fmt = fmt.replace(ret[1], (ret[1].length == 1) ? (opt[k]) : (opt[k].padStart(ret[1].length, "0")))
        };
    };
    return fmt;
}

var apiBaseUrl = "";

var getXmlHttpObj = function () {
    var xmlHttp = null;
    if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
    } else {
        try {
            xmlHttp = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
                xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {
                return null;
            }
        }
    }
    return xmlHttp;
};
//method, uri, data, timeout, cbOk, cbErr
function ajax(params) {
    var apiUrl= apiBaseUrl+params.uri;
    var xmlHttpObj = getXmlHttpObj();

    var errInfo;
    if (!xmlHttpObj) {
        errInfo = "创建请求失败";
        if (params.error) params.error(errInfo);
        return;
    }

    xmlHttpObj.open(params.method, apiUrl, true);
    xmlHttpObj.onreadystatechange = function () {
        if (xmlHttpObj.readyState == 4) {
            if (xmlHttpObj.status == 200) {
                var data = xmlHttpObj.responseText;
                if (params.success) params.success(JSON.parse(data));
                xmlHttpObj = null;
            } else {
                xmlHttpObj = null;
                //避免刷新的时候，由于abord ajax引起的错误回调
                setTimeout(function () {
                    var errInfo = "请求服务器失败,请检查你的网络是否正常";
                    if (params.error) params.error(errInfo);
                }, 16);
            }
        }
    };

    xmlHttpObj.setRequestHeader('Content-Type', "application/x-www-form-urlencoded");

    xmlHttpObj.timeout = 15000;
    xmlHttpObj.ontimeout = function (event) {
        xmlHttpObj = null;
        var errInfo = "请求服务器超时";
        if (params.error) params.error(errInfo);
    };

    if (xmlHttpObj.overrideMimeType) {
        xmlHttpObj.overrideMimeType("application/json;charset=utf-8");
    }

    xmlHttpObj.send(params.data);


}

function box(id){

    //获取DIV为‘box’的盒子

    var oBox = document.getElementById(id);

    //获取元素自身的宽度

    var L1 = oBox.offsetWidth;

    //获取元素自身的高度

    var H1 = oBox.offsetHeight;

    //获取实际页面的left值。（页面宽度减去元素自身宽度/2）

    var Left = (document.documentElement.clientWidth-L1)/2;

    //获取实际页面的top值。（页面宽度减去元素自身高度/2）

    var top = (document.documentElement.clientHeight-H1)/2;

    oBox.style.left = Left+'px';

    oBox.style.top = top+'px';

}
function GetQueryString(name) {
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
    var r = window.location.search.substr(1).match(reg); //获取url中"?"符后的字符串并正则匹配
    var context = "";
    if (r != null)
        context = r[2];
    reg = null;
    r = null;
    return context == null || context == "" || context == "undefined" ? "" : context;
}

function getOS() {
    var sUserAgent = navigator.userAgent;
//alert(navigator.platform);
    var isWin = (navigator.platform == "Win32") || (navigator.platform == "Win64") || (navigator.platform == "Windows");
    var isMac = (navigator.platform == "Mac68K") || (navigator.platform == "MacPPC") || (navigator.platform == "Macintosh") || (navigator.platform == "MacIntel");
    if (isMac) return "Mac";
    var isUnix = (navigator.platform == "X11") && !isWin && !isMac;
    if (isUnix) return "Unix";
    var isLinux = (String(navigator.platform).indexOf("Linux") > -1);
    if (isLinux) return "Linux";
    if (isWin) {
        var isWin2K = sUserAgent.indexOf("Windows NT 5.0") > -1 || sUserAgent.indexOf("Windows 2000") > -1;
        if (isWin2K) return "Win2000";
        var isWinXP = sUserAgent.indexOf("Windows NT 5.1") > -1 || sUserAgent.indexOf("Windows XP") > -1;
        if (isWinXP) return "WinXP";
        var isWin2003 = sUserAgent.indexOf("Windows NT 5.2") > -1 || sUserAgent.indexOf("Windows 2003") > -1;
        if (isWin2003) return "Win2003";
        var isWinVista= sUserAgent.indexOf("Windows NT 6.0") > -1 || sUserAgent.indexOf("Windows Vista") > -1;
        if (isWinVista) return "WinVista";
        var isWin7 = sUserAgent.indexOf("Windows NT 6.1") > -1 || sUserAgent.indexOf("Windows 7") > -1;
        if (isWin7) return "Win7";
        var isWin10 = sUserAgent.indexOf("Windows NT 10") > -1 || sUserAgent.indexOf("Windows 10") > -1;
        if (isWin10) return "Win10";
    }
    return "other";
}


function uaMatch(){

    var userAgent = navigator.userAgent,
        rMsie = /(trident.*rv:)([\w.]+)/;
    var browser;
    var version;
    var ua = userAgent.toLowerCase();

    var match = rMsie.exec(ua);
    if(match){
        return { browser : "IE", version : match[2] || "0" };
    }else{
        return { browser : "none", version : 0 };
    }

}


function getSize() //函数：获取尺寸
{
    var winWidth = 0;
    var winHeight = 0;

    //获取窗口宽度
    if (window.innerWidth)
        winWidth = window.innerWidth;
    else if ((document.body) && (document.body.clientWidth))
        winWidth = document.body.clientWidth;


    //获取窗口高度
    if (window.innerHeight)
        winHeight = window.innerHeight;
    else if ((document.body) && (document.body.clientHeight))
        winHeight = document.body.clientHeight;
    //通过深入Document内部对body进行检测，获取窗口大小
    if (document.documentElement && document.documentElement.clientHeight && document.documentElement.clientWidth)
    {
        winHeight = document.documentElement.clientHeight;
        winWidth = document.documentElement.clientWidth;
    }
    //结果输出至两个文本框
    return {
        "width":winWidth,
        "height":winHeight
    };
}
function check(val) {
    if (val < 10) {
        return ("0" + val);
    }
    else {
        return (val);
    }
}
function displayTime() {
    //获取div元素
   // var timeDiv=document.getElementById(timeDom);
    //var dateDiv=document.getElementById(dateDom);
    //获取系统当前的年、月、日、小时、分钟、毫秒
    var date = new Date();
    var year = date.getFullYear();
    var month = date.getMonth() + 1;
    var day = date.getDate();
    var hour = date.getHours();
    var minutes = date.getMinutes();
    var second = date.getSeconds();
    var timestr = year + "年" + month + "月" + day + "日  " + check(hour)
        + ":" + check(minutes) + ":" + check(second);
    //将系统时间设置到div元素中
    //timeDiv.innerHTML = check(hour)+ ":" + check(minutes) + ":" + check(second);
    $("#time").html(check(hour)+ ":" + check(minutes) + ":" + check(second));
    var monthName = ["","JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"];
    //dateDiv.innerHTML = year+"-"+monthName[month]+"-"+day;
    $(".dateItem").html('<span class="year">'+year+'</span><br/><span class="yearValue">'+monthName[month]+'<br/>'+day+'</span>');
}
//每隔1秒调用一次displayTime函数
function start(){
    window.setInterval("displayTime()",1000)//单位是毫秒
}

function htmlspecialchars(str){
    //str = str.replace(/&/g, '&amp;');


    return str;
}

var scrollAction = {x: 'undefined', y: 'undefined'}, scrollDirection;
function scrollFunc() {
    if (typeof scrollAction.x == 'undefined') {
        scrollAction.x =  document.getElementById("chatList").pageXOffset;
        scrollAction.y =  document.getElementById("chatList").pageYOffset;
    }
    var diffX = scrollAction.x -  document.getElementById("chatList").pageXOffset;
    var diffY = scrollAction.y -  document.getElementById("chatList").pageYOffset;
    if (diffX < 0) {
        // Scroll right
        scrollDirection = 'right';
    } else if (diffX > 0) {
        // Scroll left
        scrollDirection = 'left';
    } else if (diffY < 0) {
        // Scroll down
        scrollDirection = 'down';
    } else if (diffY > 0) {
        // Scroll up
        scrollDirection = 'up';
    } else {
        // First scroll event
    }
    scrollAction.x = document.getElementById("chatList").pageXOffset;
    scrollAction.y = document.getElementById("chatList").pageYOffset;
}

function timestampToTime(timestamp) {
    var date = new Date(timestamp * 1000);//时间戳为10位需*1000，时间戳为13位的话不需乘1000
    Y = date.getFullYear() + '-';
    M = (date.getMonth()+1 < 10 ? '0'+(date.getMonth()+1) : date.getMonth()+1) + '-';
    D = date.getDate() + ' ';
    h = date.getHours() + ':';
    m = date.getMinutes() + ':';
    s = date.getSeconds();
    return Y+M+D+h+m+s;
}

var colors = [
    "mediumseagreen",
    "darkseagreen",
    "yellowgreen",
    "limegreen",
    "lime",
    "chartreuse",
    "lawngreen",
    "greenyellow",
    "mediumspringgreen",
    "springgreen",
    "lightgreen",
    "palegreen",
    "aquamarine",
    "honeydew",
    "midnightblue",
    "navy",
    "darkblue",
    "darkslateblue",
    "mediumblue",
    "royalblue",
    "dodgerblue",
    "cornflowerblue",
    "deepskyblue",
    "lightskyblue",
    "lightsteelblue",
    "lightblue",
    "steelblue",
    "darkcyan",
    "cadetblue",
    "darkturquoise",
    "mediumturquoise",
    "turquoise",
    "skyblue",
    "powderblue",
    "paleturquoise",
    "lightcyan",
    "azure",
    "aliceblue",
    "aqua(cyan)"
];
