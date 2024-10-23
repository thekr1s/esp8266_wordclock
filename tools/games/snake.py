import random
import time
from PIL import Image
from pynput import keyboard
from woordklok_api import Woordklok

last_key = None
host = ('192.168.2.87', 21324)

class SnakeGame:
    def __init__(self, width=13, height=13):
        self.width = width
        self.height = height
        self.reset()

    def reset(self):
        self.snake = [(6, 6)]
        self.direction = (0, 1)  # Start moving to the right
        self.generate_food()
        self.game_over = False

    def generate_food(self):
        while True:
            self.food = (random.randint(0, self.width - 1), random.randint(0, self.height - 1))
            if self.food not in self.snake:
                break

    def update(self):
        if self.game_over:
            return

        new_head = (self.snake[0][0] + self.direction[0], self.snake[0][1] + self.direction[1])
        
        # Check for collisions with walls
        if (new_head[0] < 0 or new_head[0] >= self.width or
            new_head[1] < 0 or new_head[1] >= self.height):
            self.game_over = True
            return

        # Check for collisions with self
        if new_head in self.snake:
            self.game_over = True
            return

        # Check for food
        if new_head == self.food:
            self.snake.insert(0, new_head)  # Add new head (snake grows)
            self.generate_food()  # Generate new food
        else:
            self.snake.insert(0, new_head)
            self.snake.pop()  # Remove tail (snake moves)

    def change_direction(self, direction):
        opposite_direction = (-self.direction[0], -self.direction[1])
        if direction != opposite_direction:  # Prevent the snake from reversing
            self.direction = direction

    def get_image(self):
        image = Image.new('RGB', (self.width, self.height), 'black')
        for x, y in self.snake:
            image.putpixel((x, y), (0, 255, 0))  # Snake is green
        food_x, food_y = self.food
        image.putpixel((food_x, food_y), (255, 0, 0))  # Food is red
        return image

def on_press(key):
    global last_key
    try:
        key = key.char
        if key in ['w', 's', 'a', 'd']:
            last_key = key
    except AttributeError:
        key = str(key)
    #print(f"Last key pressed: {key}")
    
def main():
    keyboardHdl = keyboard.Listener(on_press=on_press)
    keyboardHdl.start()

    game = SnakeGame()
    direction_map = {
        'w': (0, -1),  # Up
        's': (0, 1),   # Down
        'a': (-1, 0),  # Left
        'd': (1, 0)    # Right
    }
    
    wled = Woordklok(host)
    wled.start_realtime_udp()
    print("Waiting for input (A,S,D,W)")
    while last_key == None:
        time.sleep(1)
    while not game.game_over:
        game.update()
        image = game.get_image()
        wled.send_image(image)
        time.sleep(0.3)  # Delay for game speed

        # Example input for changing direction (use non-blocking input method in actual implementation)
        direction = last_key
        game.change_direction(direction_map[direction])

    keyboardHdl.stop()
    print("Game Over")

if __name__ == "__main__":
    main()