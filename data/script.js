/**
  Curiosity Controller
  Drivable scale model of the Mars Science Laboratory, AKA, "Curiosity"
  Based on code examples by:
  Rui Santos & Sara Santos - Random Nerd Tutorials: 
  https://RandomNerdTutorials.com/esp32-websocket-server-sensor/
  Yoann Moinet's joystick library (NippleJS):
  https://github.com/yoannmoinet/nipplejs
 */

(function () {
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  var currentPosition = {
    vertical: 0,
    horizontal: 0,
  };

  // Init web socket when the page loads

  function initWebSocket() {
    console.log("Trying to open a WebSocket connection…");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;

    manager
      .on("added", function (evt, nipple) {
        nipple.on("start move", function (evt) {
          currentPosition.horizontal = nipple.frontPosition.x / 150.0;
          currentPosition.vertical = nipple.frontPosition.y / 150.0;
          console.log(currentPosition);

          try {
            websocket.send(
              JSON.stringify({
                horizontal: currentPosition.horizontal,
                vertical: currentPosition.vertical,
                timestamp: Date.now(),
              })
            );
          } catch (e) {

          }

        });
      })
      .on("end", function () {
        currentPosition = {
          horizontal: 0,
          vertical: 0,
        };
        websocket.send(
          JSON.stringify({
            horizontal: 0,
            vertical: 0,
            timestamp: Date.now(),
          })
        );
      })
      .on("removed", function (evt, nipple) {
        nipple.off("start move end");
      });
  }

  window.addEventListener("load", initWebSocket);

  // When websocket is established, call the getReadings() function
  function onOpen(event) {
    console.log("WebSocket Connection opened");
  }

  function onClose(event) {
    console.log("Connection closed");
    setTimeout(initWebSocket, 2000);
  }

  // Function that receives the message from the ESP32 with the readings
  function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++) {
      var key = keys[i];
      document.getElementById(key).innerHTML = myObj[key];
    }
  }
})();
