// ---------------------------------------------------------------------------
// PebbleKit JS: 位置情報 + OpenWeatherMap から天気を取得して時計へ送信する
// ---------------------------------------------------------------------------
var API_KEY = 'YOUR_OPENWEATHERMAP_API_KEY';
//             ↑ ここに実際のキーを貼り付ける (https://openweathermap.org/api)

var UPDATE_INTERVAL_MS = 30 * 60 * 1000; // 30分ごとに更新
var FORECAST_AHEAD_SEC = 6 * 60 * 60;    // 約6時間後の予報を使う

function xhrRequest(url, callback, errorCallback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    if (xhr.status === 200) {
      callback(xhr.responseText);
    } else {
      errorCallback('HTTP ' + xhr.status);
    }
  };
  xhr.onerror = function () {
    errorCallback('network error');
  };
  xhr.open('GET', url);
  xhr.send();
}

// 3時間刻みの予報リストから目標時刻に最も近いエントリを選ぶ
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
  var query = 'lat=' + lat + '&lon=' + lon + '&units=metric&appid=' + API_KEY;
  var currentUrl = 'https://api.openweathermap.org/data/2.5/weather?' + query;
  var forecastUrl = 'https://api.openweathermap.org/data/2.5/forecast?' + query;

  xhrRequest(currentUrl, function (currentText) {
    var current = JSON.parse(currentText);

    xhrRequest(forecastUrl, function (forecastText) {
      var entry = pickForecastEntry(JSON.parse(forecastText).list);

      Pebble.sendAppMessage({
        'TEMPERATURE': Math.round(current.main.temp),
        'CONDITIONS': current.weather[0].main,
        'PRESSURE': Math.round(current.main.pressure),
        'FC_TEMPERATURE': Math.round(entry.main.temp),
        'FC_CONDITIONS': entry.weather[0].main,
        'FC_PRESSURE': Math.round(entry.main.pressure)
      }, function () {
        console.log('Weather sent to watch.');
      }, function (e) {
        console.log('Error sending weather to watch: ' + JSON.stringify(e));
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
