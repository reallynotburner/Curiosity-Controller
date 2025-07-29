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
  var currentPosition = {
    vertical: 0,
    horizontal: 0,
  };
  var calibrating = false;
  // I've numbered the wheels starting with '1' so there are 7 elements in this array
  var calibrationStatus = [false, false, false, false, false, false, false];

  // Init web socket when the page loads
  function initWebSocket() {
    try {
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
      websocketOpen && manager
        .on("added", function (evt, nipple) {
          nipple.on("start move", function (evt) {
            currentPosition.horizontal = nipple.frontPosition.x / 150.0;
            currentPosition.vertical = nipple.frontPosition.y / 150.0;
            websocketOpen &&
              websocket.send(
                JSON.stringify({
                  horizontal: currentPosition.horizontal,
                  vertical: currentPosition.vertical,
                  calibrating,
                  calibrationStatus,
                  timestamp: Date.now(),
                })
              );
          });
        })
        .on("end", function () {
          currentPosition = {
            horizontal: 0,
            vertical: 0,
          };
          websocketOpen &&
            websocket.send(
              JSON.stringify({
                horizontal: 0,
                vertical: 0,
                calibrating,
                calibrationStatus,
                timestamp: Date.now(),
              })
            );
        })
        .on("removed", function (evt, nipple) {
          nipple.off("start move end");
        });
    } catch (e) {}
  }

  function setCalButtonStatus (state) {
    let calibrationButton = document.getElementById("calbtn");
    if (state) {
      calibrationButton.classList.add('active');
    } else {
      calibrationButton.classList.remove('active');
    }
  }

  window.addEventListener("load", initWebSocket);
  window.onhashchange = (evt) => {
    calibrationStatus.fill(false); // reset the state
    calibrating = true;
    switch (window.location.hash) {
      case "#leftfrontcalibrate":
        calibrationStatus[1] = true;
        setCalButtonStatus(true);
        break;
      case "#rightfrontcalibrate":
        calibrationStatus[2] = true;
        setCalButtonStatus(true);
        break;
      case "#leftrearcalibrate":
        calibrationStatus[5] = true;
        setCalButtonStatus(true);
        break;
      case "#rightrearcalibrate":
        calibrationStatus[6] = true;
        setCalButtonStatus(true);
        break;
      case "#move":
        calibrating = false;
        setCalButtonStatus(false);
        break;
      default:
        calibrating = false;
        setCalButtonStatus(false);
        break;
    }
  }

  // When websocket is established, call the getReadings() function
  function onOpen() {
    websocketOpen = true;
    console.log("WebSocket Connection opened");
  }

  function onClose(event) {
    console.log("Connection closed");
    websocketOpen = false;
    setTimeout(initWebSocket, 2000);
  }

  // Function that receives the message from the ESP32 with the readings
  function onMessage(event) {
    console.log(event.data);
  }
})();
