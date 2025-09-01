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
  var websocketOpen = false;
  var currentJoystickPosition = {
    vertical: 0,
    horizontal: 0,
  };

  var calibrating = false;
  var spin = false;
  // I've numbered the axes starting with '1', so zero is no-axis selected
  var calibrationAxis = 0;
  var calibrationPoint = "middle"; // "start || middle || end"

  // Init web socket when the page loads
  function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
  }

  function initJoystick() {
    manager
      .on("added", function (_, nipple) {
        nipple.on("start move", function () {
          calibrating = calibrationAxis ? true : false;
          currentJoystickPosition.horizontal = nipple.frontPosition.x / 150.0;
          currentJoystickPosition.vertical = nipple.frontPosition.y / -150.0;
          websocketOpen &&
            websocket.send(
              JSON.stringify({
                horizontal: currentJoystickPosition.horizontal,
                vertical: currentJoystickPosition.vertical,
                calibrating,
                calibrationAxis,
                calibrationPoint,
                spin,
                timestamp: Date.now(),
              })
            );
        });
      })
      .on("end", function (_, nipple) {
        calibrating = false;
        if (calibrationAxis) {
          console.log(
            "ENDED WHILE CALIBRATING!",
            currentJoystickPosition.horizontal,
            calibrating
          );
        }
        currentJoystickPosition.horizontal = calibrationAxis ? nipple.frontPosition.x / 150.0 : 0;
        currentJoystickPosition.vertical = calibrationAxis ? nipple.frontPosition.y / -150.0 : 0;
        websocketOpen &&
          websocket.send(
            JSON.stringify({
              horizontal: currentJoystickPosition.horizontal,
              vertical: currentJoystickPosition.vertical,
              calibrating,
              calibrationAxis,
              calibrationPoint,
              spin,
              timestamp: Date.now(),
            })
          );
      })
      .on("removed", function (evt, nipple) {
        nipple.off("start move end");
        if (calibrationAxis) {
          console.log(
            "REMOVED WHILE CALIBRATING!",
            currentJoystickPosition.horizontal
          );
        }
        currentJoystickPosition = {
          horizontal: 0,
          vertical: 0,
        }
        calibrating = false;
        websocketOpen &&
          websocket.send(
            JSON.stringify({
              horizontal: 0,
              vertical: 0,
              calibrating,
              calibrationAxis,
              calibrationPoint,
              spin,
              timestamp: Date.now(),
            })
          );
      });
  }

  window.addEventListener("load", () => {
    initWebSocket();
    initJoystick();
  });

  window.onhashchange = (evt) => {
    calibrationAxis = 0; // reset the state
    switch (window.location.hash) {
      case "#leftfrontcalibrate":
        calibrationAxis = 1;
        spin = false;
        break;
      case "#rightfrontcalibrate":
        calibrationAxis = 2;
        spin = false;
        break;
      case "#leftrearcalibrate":
        calibrationAxis = 5;
        spin = false;
        break;
      case "#rightrearcalibrate":
        calibrationAxis = 6;
        spin = false;
        break;
      case "#move":
        spin = false;
        websocketOpen &&
          websocket.send(
            JSON.stringify({
              horizontal: 0,
              vertical: 0,
              calibrating,
              calibrationAxis,
              calibrationPoint,
              spin,
              timestamp: Date.now(),
            })
          );
        break;
      case "#spin":
        spin = true;
        websocketOpen &&
          websocket.send(
            JSON.stringify({
              horizontal: 0,
              vertical: 0,
              calibrating,
              calibrationAxis,
              calibrationPoint,
              spin,
              timestamp: Date.now(),
            })
          );
        break;
      default:
        spin = false;
        break;
    }
  };

  // When websocket is established, call the getReadings() function
  function onOpen() {
    websocketOpen = true;
  }

  function onClose(event) {
    websocketOpen = false;
    setTimeout(initWebSocket, 2000);
  }

  // Function that receives the message from the ESP32 with the readings
  function onMessage(event) {
    console.log("message from ESP32", event.data);
  }
})();
