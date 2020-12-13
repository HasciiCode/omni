function ajax_template(){
    const method    = "GET";
    const url       = "style.css";
    const data      = null;      

    var xhr = new XMLHttpRequest();
    xhr.open(method,url,true);
    xhr.send(data);

    xhr.onreadystatechange = function(){
        if(xhr.readyState === 4){
            if(xhr.status === 200){ //通信に成功（レスポンスコード:200）したときに処理される
                //データを受け取ったの処理をこの中に書く
                console.log(xhr.responseText);
            }
        }else{
            //xhr.readyStateはajaxの処理プロセスの番号が返ってくるもの。
            //4がかえってきたときにデータを送信している。（データの送信に失敗しても4は返ってくる）
        }
    }
}

//CSVファイルを二次元配列に変換する
function CSVtoArray(csv_data){
    var csv_line = csv_data.split("\n");
    var csv_array = new Array();
    
    for (var i=0;i<csv_line.length;i++){
        csv_array[i] = csv_line[i].split(",");  
    }

    return csv_array;
}

//サーバーで取得したSSIDを受け取って表示する
function GetNetworks(data){
    const method    = "GET";
    const url       = "/api/network.csv";   

    var xhr = new XMLHttpRequest();
    xhr.open(method,url,true);
    xhr.send(data);

    xhr.onreadystatechange = function(){
        if(xhr.readyState === 4){
            if(xhr.status === 200){
                var network_array = [];
                var wifi_select = document.getElementById("wifi-select");
                    
                network_array = CSVtoArray(xhr.responseText);

                for (var i=0;i<network_array[0].length;i++){
                    var additional_option = document.createElement("option");
                
                    additional_option.text  = network_array[0][i];
                    additional_option.value = network_array[0][i];

                    wifi_select.appendChild(additional_option);                            
                }
            }
        }else{
            //xhr.readyStateはajaxの処理プロセスの番号が返ってくるもの。
            //4がかえってきたときにデータを送信している。（データの送信に失敗しても4は返ってくる）
        }
    }
}

function SendWiFiSetting(data){
    const method    = "POST";
    const url       = "regist_wifi";   

    var xhr = new XMLHttpRequest();
    xhr.open(method,url,true);
    xhr.send(data);

    xhr.onreadystatechange = function(){
        if(xhr.readyState === 4){
            if(xhr.status === 200){ //通信に成功（レスポンスコード:200）したときに処理される
                //データを受け取ったの処理をこの中に書く
                console.log(xhr.responseText);
            }
        }else{
            //xhr.readyStateはajaxの処理プロセスの番号が返ってくるもの。
            //4がかえってきたときにデータを送信している。（データの送信に失敗しても4は返ってくる）
        }
    }
}

function wifi_setting_app(){
    const submit_form = document.getElementById("wifi-password");
}

//HTMLを帯びだし終わった後の処理
window.onload = function(){
    //alert("ページが読み込まれました");
    GetNetworks();
    wifi_setting_app();
}


window.document.onkeydown = function(event){
    if (event.keyCode === 13){
        var obj = document.activeElement;
        console.log(obj.id);
        try{
            if(obj.id === "wifi-password"){
                const wifi_ssid = document.getElementById("wifi-select");
                const wifi_pass = document.getElementById("wifi-password");
                var wifi_json = {
                    "wifi_ssid": wifi_ssid.value,
                    "wifi_pass": wifi_pass.value
                }
                console.log(wifi_json);
                SendWiFiSetting(wifi_json);
            }else{
                obj.nextElementSibling.focus();
            }            
        }catch(TypeError){
            console.log("CATCHED");
        }
    }
}