// ---------------------------------------------------------------------------
// PebbleKit JS: 位置情報 + OpenWeatherMap から天気を取得して時計へ送信する
// ---------------------------------------------------------------------------
var API_KEY = 'YOUR_OPENWEATHERMAP_API_KEY';
//             ↑ ここに実際のキーを貼り付ける (https://openweathermap.org/api)

var UPDATE_INTERVAL_MS = 30 * 60 * 1000; // 30分ごと
var FORECAST_AHEAD_SEC = 6 * 60 * 60;    // 約6時間後の予報を使う

function xhrRequest(url, type, callback, errorCallback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    if (xhr.status === 200) {
      callback(xhr.responseText);
    } else if (errorCallback) {
      errorCallback('HTTP ' + xhr.status);
    }
  };
  xhr.onerror = function () {
    if (errorCallback) errorCallback('network error');
  };
  xhr.open(type, url);
  xhr.send();
}

function pickForecastEntry(list) {
  var target = Date.now() / 1000 + FORECAST_AHEAD_SEC;
  var best = list[0];
  var bestDiff = Math.abs(best.dt - target);
  for (var i = 1; i < list.length; i++) {
    var diff = Math.abs(list[i].dt - target);
    if (diff < bestDiff) {
      best = list[i];
      bestDiff = diff;
    }
  }
  return best;
}

function fetchWeather(lat, lon) {
  var currentUrl = 'https://api.openweathermap.org/data/2.5/weather?lat=' +
    lat + '&lon=' + lon + '&units=metric&appid=' + API_KEY;
  var forecastUrl = 'https://api.openweathermap.org/data/2.5/forecast?lat=' +
    lat + '&lon=' + lon + '&units=metric&appid=' + API_KEY;

  xhrRequest(currentUrl, 'GET', function (currentText) {
    var current = JSON.parse(currentText);
    var temperature = Math.round(current.main.temp);
    var conditions = current.weather[0].main;
    var pressure = Math.round(current.main.pressure);

    xhrRequest(forecastUrl, 'GET', function (forecastText) {
      var forecast = JSON.parse(forecastText);
      var entry = pickForecastEntry(forecast.list);
      var fcTemperature = Math.round(entry.main.temp);
      var fcConditions = entry.weather[0].main;
      var fcPressure = Math.round(entry.main.pressure);

      Pebble.sendAppMessage({
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions,
        'KEY_PRESSURE': pressure,
        'KEY_FC_TEMPERATURE': fcTemperature,
        'KEY_FC_CONDITIONS': fcConditions,
        'KEY_FC_PRESSURE': fcPressure
      }, function () {
        console.log('Weather sent to watch.');
      }, function (e) {
        console.log('Error sending weather to watch: ' + e.error.message);
      });
    }, function (err) {
      console.log('Forecast request failed: ' + err);
    });
  }, function (err) {
    console.log('Current weather request failed: ' + err);
  });
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    function (pos) {
      fetchWeather(pos.coords.latitude, pos.coords.longitude);
    },
    function () {
      console.log('Location request failed.');
    },
    { timeout: 15000, maximumAge: 60000 }
  );
}

Pebble.addEventListener('ready', function () {
  console.log('PebbleKit JS ready.');
  getWeather();
  setInterval(getWeather, UPDATE_INTERVAL_MS);
});

Pebble.addEventListener('appmessage', function () {
  getWeather();
});
