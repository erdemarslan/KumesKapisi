let saatInterval;
function saatleriGoster() {
    const bugun = new Date();
    let tarih = bugun.getDate() + "." + (bugun.getMonth()+1) + "." + bugun.getFullYear() + " " + bugun.getHours() + ":" + bugun.getMinutes() + ":" + bugun.getSeconds();
    $("#bilgisayar_saati").html(tarih);

    $.ajax({
        type: "GET",
        url: "/getModuleDateTime",
        success: function(data){
            $("#cihaz_saati").html(data);
        },
        error: function (request, status, error) {
            alert(request.responseText);
        }
    });
}

function otomatikCalismaAktifmi() {
    $.ajax({
        type: "GET",
        url: "/isAutoRunActive",
        success : function (data) {
            //
            if(data == "evet") {
                // açık
                $("#autoOnOf").prop('checked', true);
                $("#autoOnOffLabel").html("Açık");
            } else {
                // açık değil
                $("#autoOnOf").prop('checked', false);
                $("#autoOnOffLabel").html("Kapalı");
            }
        },
        error: function (request, status, error) {
            alert(request.responseText);
        }
    });
}

function setAutoOnOff(veri, durum) {
    $.ajax({
        type: "POST",
        url: "/setAutoRun",
        data : {auto : veri},
        success: function (data) {
            if (data == "OK") {
                $("#autoOnOffLabel").html(durum);
            } else {
                alert("Değişiklik yapılamadı...");
                // butonu eski haline döndürelim....
                $(this).prop('checked', !$(this).prop('checked'));
            }
        },
        error: function (request, status, error) {
            alert(request.responseText);
            $(this).prop('checked', !$(this).prop('checked'));
        }
    });
}
$(document).ready(function() {
    console.log("pencere yüklendi....");
    // saatleri bir kereliğine çalıştır.
    saatleriGoster();
    // modülden otomatik çalışma aktif mi onu sor...
    otomatikCalismaAktifmi();
    // saatleri sürekli çalışır hale getir...
    saatInterval = setInterval(saatleriGoster, 1000);
});

// telefon/bilgisayar saatini modüle kaydeder...
$("#saveDateTimeToModule").click(function(){
    //const months = ["Jan", "Feb", "Mar","Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
    const bugun = new Date();
    //const newDate = months[bugun.getMonth()] + " " + bugun.getDate() + " " + bugun.getFullYear();
    //let hours = bugun.getHours() < 10 ? "0" + bugun.getHours() : bugun.getHours();
    //let minutes = bugun.getMinutes() < 10 ? "0" + bugun.getMinutes() : bugun.getMinutes();
    //let seconds = bugun.getSeconds() < 10 ? "0" + bugun.getSeconds() : bugun.getSeconds();
    //const newTime = hours + ":" + minutes + ":" + seconds;

    $.ajax({
        type: "POST",
        url: "/saveDateTimeToModule",
        data: {yil: bugun.getFullYear(), ay: (bugun.getMonth()+1), gun: bugun.getDate(), saat: bugun.getHours(), dakika: bugun.getMinutes(), saniye: bugun.getSeconds()},
        success: function (data) {
            saatleriGoster();
            alert("Saat güncellendi...");
        },
        error: function (request, status, error) {
            alert(request.responseText);
        }
    });


});

// otomatik çalışmayı başlatır durdurur...
$("#autoOnOf").change(function(){
    const veri = $(this).prop("checked") ? "1" : "0";
    const durum = $(this).prop('checked') ? "Açık" : "Kapalı";
    setAutoOnOff(veri, durum);
});

$("#button_up").click(function () {
    clearInterval(saatInterval);
    setAutoOnOff(0, "Kapalı");
    $("#autoOnOf").prop("checked", false);
    $.ajax({
        type: "POST",
        url : "/moveDoor",
        data : { yon : 1},
        success : function (data) {
            if(data != "OK") {
                alert("Kapağı yukarıya kaldırma ile ilgili bir sorun var...");
            }
            saatInterval = setInterval(saatleriGoster, 1000);
        },
        error: function (request, status, error) {
            saatInterval = setInterval(saatleriGoster, 1000);
            alert(request.responseText);
        }
    });
});

$("#button_down").click(function () {
    clearInterval(saatInterval);
    setAutoOnOff(0, "Kapalı");
    $("#autoOnOf").prop("checked", false);
    $.ajax({
        type: "POST",
        url : "/moveDoor",
        data : { yon : 0},
        success : function (data) {
            if(data != "OK") {
                alert("Kapağı aşağıya indirme ile ilgili bir sorun var...");
            }
            saatInterval = setInterval(saatleriGoster, 1000);
        },
        error: function (request, status, error) {
            saatInterval = setInterval(saatleriGoster, 1000);
            alert(request.responseText);
        }
    });
});