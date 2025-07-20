// Frank Poth 10/03/2017

(function () {
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;

  // Init web socket when the page loads

  function initWebSocket() {
    console.log("Trying to open a WebSocket connection…");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
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
  var Button, Stick, controller, display, game;

  // basically a rectangle, but it's purpose here is to be a button:
  Button = function (x, y, width, height, color, name) {
    this.active = false;
    this.color = color;
    this.height = height;
    this.width = width;
    this.x = x;
    this.y = y;
    this.name = name;
    this.horizontal = 0.0;
    this.vertical = 0.0;
  };

  Button.prototype = {
    // returns true if the specified point lies within the rectangle:
    containsPoint: function (x, y) {
      // if the point is outside of the rectangle return false:
      if (
        x < this.x ||
        x > this.x + this.width ||
        y < this.y ||
        y > this.y + this.width
      ) {
        this.horizontal = 0.0;
        this.vertical = 0.0;
        return false;
      }

      // if the point is inside the rectangle, calculate the horizontal and vertical values:
      this.horizontal = 2*(x - this.x) / this.width - 1.0;
      this.vertical = -2*(y - this.y) / this.height + 1.0;
      
      return true;
    },
  };


  // handles everything to do with user input:
  controller = {
    buttons: {
      joystick: new Button(50, 50, 300, 300, "#007700", "joystick"),
    },

    testButtons: function (target_touches) {
      var index, touch;

      // loop through all buttons:
      for (let button in this.buttons) {
        if (controller.buttons.hasOwnProperty(button)) {
          // Important to filter out inherited properties
          this.buttons[button].active = false;

          // loop through all touch objects:
          for (index = target_touches.length - 1; index > -1; --index) {
            touch = target_touches[index];

            // make sure the touch coordinates are adjusted for both the canvas offset and the scale ratio of the buffer and output canvases:
            if (
              this.buttons[button].containsPoint(
                (touch.clientX - display.bounding_rectangle.left) *
                  display.buffer_output_ratio,
                (touch.clientY - display.bounding_rectangle.top) *
                  display.buffer_output_ratio
              )
            ) {
              this.buttons[button].active = true;
              break; // once the button is active, there's no need to check if any other points are inside, so continue
            }
          }
        }
      }

      websocket.send(
        JSON.stringify({
          horizontal: this.buttons.joystick.horizontal ? this.buttons.joystick.horizontal : 0,
          vertical: this.buttons.joystick.active ? this.buttons.joystick.vertical : 0,
          timestamp: Date.now(),
        })
      );

      display.message.innerHTML =
        "touches: " + event.targetTouches.length + "<br> ";

      // if (this.buttons.joystick.active) {
        display.message.innerHTML +="H:"
        display.message.innerHTML += this.buttons.joystick.horizontal.toFixed(2);
        display.message.innerHTML += " V:";
        display.message.innerHTML += this.buttons.joystick.vertical.toFixed(2);
    //  }
    },

    touchEnd: function (event) {
      event.preventDefault();
      controller.testButtons(event.targetTouches);
    },

    touchMove: function (event) {
      event.preventDefault();
      controller.testButtons(event.targetTouches);
    },

    touchStart: function (event) {
      event.preventDefault();
      controller.testButtons(event.targetTouches);
    },
  };

  // handles everything to do with displaying graphics on the screen:
  display = {
    // the buffer is used to scale the applications graphics to fit the screen:
    buffer: document.createElement("canvas").getContext("2d"),
    // the on screen canvas context that we will be drawing to:
    output: document.querySelector("canvas").getContext("2d"),
    // the p element for text output:
    message: document.querySelector("p"),

    // the ratio in size between the buffer and output canvases used to scale user input coordinates:
    buffer_output_ratio: 1,
    // the bounding rectangle of the output canvas used to determine the location of user input on the output canvas:
    bounding_rectangle: undefined,

    // clears the display canvas to the specified color:
    clear: function (color) {
      this.buffer.fillStyle = color || "#000000";
      this.buffer.fillRect(
        0,
        0,
        this.buffer.canvas.width,
        this.buffer.canvas.height
      );
    },

    // renders the buffer to the output canvas:
    render: function () {
      this.output.drawImage(
        this.buffer.canvas,
        0,
        0,
        this.buffer.canvas.width,
        this.buffer.canvas.height,
        0,
        0,
        this.output.canvas.width,
        this.output.canvas.height
      );
    },

    // renders the buttons:
    renderButtons: function (buttons) {
      this.buffer.fillStyle = "#202830";
      this.buffer.fillRect(
        0,
        150,
        this.buffer.canvas.width,
        this.buffer.canvas.height
      );

      // for (index = buttons.length - 1; index > -1; --index) {
      for (let button in buttons) {
        if (buttons.hasOwnProperty(button)) {
          button = buttons[button];

          this.buffer.fillStyle = button.color;
          this.buffer.fillRect(button.x, button.y, button.width, button.height);
        }
      }
    },

    // renders a square:
    renderSquare: function (square) {
      this.buffer.fillStyle = square.color;
      this.buffer.fillRect(square.x, square.y, square.width, square.height);
    },

    // just keeps the output canvas element sized appropriately:
    resize: function (event) {
      display.output.canvas.width = Math.floor(
        document.documentElement.clientWidth - 32
      );

      if (display.output.canvas.width > document.documentElement.clientHeight) {
        display.output.canvas.width = Math.floor(
          document.documentElement.clientHeight
        );
      }

      display.output.canvas.height = display.output.canvas.width;
      // display.output.canvas.height = Math.floor(
      //   display.output.canvas.width * 0.6875
      // );

      // these next two lines are used for adjusting and scaling user touch input coordinates:
      display.bounding_rectangle =
        display.output.canvas.getBoundingClientRect();

      display.buffer_output_ratio =
        display.buffer.canvas.width / display.output.canvas.width;
    },
  };

  // handles game logic:
  game = {
    loop: function (time_stamp) {
      if (controller.buttons.joystick.active) {
        
      }

      display.clear("#303840");


      display.renderButtons(controller.buttons);
      
      display.renderSquare(game.square);

      display.render();

      window.requestAnimationFrame(game.loop);
    },

    square: {
      color: "#777777",
      height: 32,
      jumping: true,
      velocity_x: 0,
      velocity_y: 0,
      width: 32,
      x: 0,
      y: 0,
    },
  };

  // initialize the application

  // size the buffer:
  display.buffer.canvas.height = 360;
  display.buffer.canvas.width = 360;

  window.addEventListener("change", display.resize);

  // setting passive:false allows you to use preventDefault in event listeners:
  display.output.canvas.addEventListener("touchend", controller.touchEnd, {
    passive: false,
  });
  display.output.canvas.addEventListener("touchmove", controller.touchMove, {
    passive: false,
  });
  display.output.canvas.addEventListener("touchstart", controller.touchStart, {
    passive: false,
  });

  // make sure the display canvas is the appropriate size on the screen:
  display.resize();

  // start the game loop:
  game.loop();
})();
