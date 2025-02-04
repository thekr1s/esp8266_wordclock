#ChatGpt input:
#Create a Pong game on a 13x13 RGB LED display using Python, we'll encapsulate the game engine in its own class. The display is updated bij import Woordklok from woordklok_api and Creating the instance by calling wled = Woordklok(('192.168.2.87', 21324)) in the main function. wled.send_image(image) update the display

import time
from PIL import Image, ImageDraw
from wledmx import WledSend
from input import get_parsed_args

class PongGame:
    def __init__(self, display):
        self.width = display.columns
        self.height = display.rows
        self.paddle_height = 3
        self.ball_pos = [self.width // 2, self.height // 2]
        self.ball_dir = [1, 1]
        self.paddle1_pos = [0, self.height // 2 - 1]
        self.paddle2_pos = [self.width - 1, self.height // 2 - 1]
        self.paddle_speed = 1
        self.ball_speed = 0.3
        self.display = display

    def create_image(self):
        image = Image.new("RGB", (self.width, self.height), "black")
        draw = ImageDraw.Draw(image)
        return image, draw

    def update_display(self):
        image, draw = self.create_image()

        # Draw paddles
        for i in range(self.paddle_height):
            draw.point((self.paddle1_pos[0], self.paddle1_pos[1] + i), fill="white")
            draw.point((self.paddle2_pos[0], self.paddle2_pos[1] + i), fill="white")

        # Draw ball
        draw.point((self.ball_pos[0], self.ball_pos[1]), fill="white")

        # Send the image to the display
        self.display.send_image(image)

    def move_paddles(self):
        # Simple AI to move paddles up and down
        if self.ball_pos[1] < self.paddle1_pos[1]:
            self.paddle1_pos[1] = max(self.paddle1_pos[1] - self.paddle_speed, 0)
        elif self.ball_pos[1] > self.paddle1_pos[1] + self.paddle_height - 1:
            self.paddle1_pos[1] = min(self.paddle1_pos[1] + self.paddle_speed, self.height - self.paddle_height)
        
        if self.ball_pos[1] < self.paddle2_pos[1]:
            self.paddle2_pos[1] = max(self.paddle2_pos[1] - self.paddle_speed, 0)
        elif self.ball_pos[1] > self.paddle2_pos[1] + self.paddle_height - 1:
            self.paddle2_pos[1] = min(self.paddle2_pos[1] + self.paddle_speed, self.height - self.paddle_height)

    def move_ball(self):
        self.ball_pos[0] += self.ball_dir[0]
        self.ball_pos[1] += self.ball_dir[1]

        # Ball collision with top and bottom walls
        if self.ball_pos[1] <= 0 or self.ball_pos[1] >= self.height - 1:
            self.ball_dir[1] *= -1

        # Ball collision with paddles
        if (self.ball_pos[0] == self.paddle1_pos[0] + 1 and self.paddle1_pos[1] <= self.ball_pos[1] <= self.paddle1_pos[1] + self.paddle_height - 1) or \
           (self.ball_pos[0] == self.paddle2_pos[0] - 1 and self.paddle2_pos[1] <= self.ball_pos[1] <= self.paddle2_pos[1] + self.paddle_height - 1):
            self.ball_dir[0] *= -1

        # Ball out of bounds
        if self.ball_pos[0] < 0 or self.ball_pos[0] > self.width - 1:
            self.ball_pos = [self.width // 2, self.height // 2]  # Reset ball position
            self.ball_dir = [1, 1]  # Reset ball direction

    def run(self):
        while True:
            self.move_paddles()
            self.move_ball()
            self.update_display()
            time.sleep(self.ball_speed)

def playpong():
    args = get_parsed_args()
    wled = WledSend((args.host, args.port), args.columns, args.rows, args.leds_per_pixel)
    game = PongGame(wled)
    game.run()

if __name__ == "__main__":
	playpong()
