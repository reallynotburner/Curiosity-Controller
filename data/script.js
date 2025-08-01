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
  // I've numbered the wheels starting with '1' so there are 7 elements in this array
  var calibrationAxis = [false, false, false, false, false, false, false];
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
      .on("added", function (evt, nipple) {
        nipple.on("start move", function (evt) {
          calibrating = calibrationAxis.some(axis => axis);
          currentJoystickPosition.horizontal = nipple.frontPosition.x / 150.0;
          currentJoystickPosition.vertical = nipple.frontPosition.y / 150.0;
          websocketOpen &&
            websocket.send(
              JSON.stringify({
                horizontal: currentJoystickPosition.horizontal,
                vertical: currentJoystickPosition.vertical,
                calibrating,
                calibrationAxis,
                calibrationPoint,
                timestamp: Date.now(),
              })
            );
        });
      })
      .on("end", function () {
        calibrating = false;
        currentJoystickPosition = {
          horizontal: 0,
          vertical: 0,
        };
        websocketOpen &&
          websocket.send(
            JSON.stringify({
              horizontal: 0,
              vertical: 0,
              calibrating,
              calibrationAxis,
              calibrationPoint,
              timestamp: Date.now(),
            })
          );
      })
      .on("removed", function (evt, nipple) {
        nipple.off("start move end");
      });
  }

  function setCalButtonStatus(state) {
    let calibrationButton = document.getElementById("calbtn");
    if (state) {
      calibrationButton.classList.add("active");
    } else {
      calibrationButton.classList.remove("active");
    }
  }

  window.addEventListener("load", () => {
    initWebSocket();
    initJoystick();
  });
  
  window.onhashchange = (evt) => {
    calibrationAxis.fill(false); // reset the state
    calibrating = true;
    switch (window.location.hash) {
      case "#leftfrontcalibrate":
        calibrationAxis[1] = true;
        break;
      case "#rightfrontcalibrate":
        calibrationAxis[2] = true;
        break;
      case "#leftrearcalibrate":
        calibrationAxis[5] = true;
        break;
      case "#rightrearcalibrate":
        calibrationAxis[6] = true;
        break;
      case "#move":
        calibrationAxis.fill(false);
        calibrating = false;
        break;
      default:
        calibrationAxis.fill(false);
        calibrating = false;
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
