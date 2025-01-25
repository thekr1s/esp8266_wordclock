import os
import time
import random
from PIL import Image, ImageDraw
import sys
from wledmx import WledSend
from input import get_parsed_args, get_resource_dir, KeyboardLinuxV2
from curses import wrapper as cursus_wrapper
from curses import error as cursus_error

class SnakePlayer:
    def __init__(self, idx, name, get_dir):
        self.name = name
        self.idx = idx
        self.get_dir = get_dir
        self.reset()
    def reset(self):
        if self.idx == 0:
            self.snake = [(2, 1), (1, 1), (0, 1)]
            self.snakedir = (1, 0)
            self.color = [(40, 200, 40), (200, 255, 200)]
            self.last_update = time.time()
        elif self.idx == 1:
            self.snake = [(0, 2), (1, 2), (2, 2)]
            self.snakedir = (-1, 0)
            self.color = [(255, 102, 178), (255, 204, 229)]
            self.last_update = time.time()
        elif self.idx == 2:
            self.snake = [(3, 3), (3, 2), (3, 1)]
            self.snakedir = (0, 1)
            self.color = [(0, 120, 204), (153, 204, 255)]
            self.last_update = time.time()
        else:
            self.snake = [(4, 1), (4, 2), (4, 3)]
            self.snakedir = (0, -1)
            self.color = [(204, 204, 0), (255, 255, 204)]
            self.last_update = time.time()
        self.delay = 0.5

class SnakeGame:
    def __init__(self, width, height, players):
        self.width = width
        self.height = height
        self.nr_of_players = len(players)
        self.players = []
        for idx, player in enumerate(players):
            self.players.append(SnakePlayer(idx, player["name"], player["input"]))
        self.reset()

    def reset(self):
        for player in self.players:
            player.reset()
        self.place_apple()
        self.state = 'playing'

    def get_next_head(self, player):
        nexthead = list(player.snake[0])
        nexthead[0] = nexthead[0] + player.snakedir[0] #increment X
        nexthead[1] = nexthead[1] + player.snakedir[1] #increment Y
        nexthead[0] = nexthead[0] % self.width
        nexthead[1] = nexthead[1] % self.height
        return tuple(nexthead)
    
    def check_collision(self, snake):
        if snake[0] in snake[1:]:
            return True
        else:
            return False
        
    def place_apple(self):
        positions = [tuple([y, x]) for x in range(self.width) for y in range(self.height)]
        # remove apple possitions that are already snakes
        for player in self.players:
            for part in player.snake:
                if part in positions:
                    positions.remove(part)
        if len(positions) == 0:
            self.state = 'finished'
        else:
            self.apple = random.choice(positions)

    def process_tick(self):
        screen_update = False
        time_now = time.time()
        for player in self.players:
            if time_now < player.last_update + player.delay:
                continue
            else:
                player.last_update = time_now
            direction = player.get_dir()
            if direction is not None:
                opposite_direction = (-direction[0], -direction[1])
                if direction != opposite_direction:  # Prevent the snake from reversing
                    player.snakedir = direction
            nexthead = self.get_next_head(player)
            if nexthead == self.apple:
                player.snake = [nexthead] + player.snake
                self.place_apple()
            else:
                player.snake = [nexthead] + player.snake[:-1]
            if self.check_collision(player.snake):
                self.state = 'gameover ' + player.name 
            player.delay = 0.5 - min((len(player.snake) * 0.03), 0.45) # Sleep at least 0.05
            screen_update = True
        return screen_update
    
    def get_image(self):
        image = Image.new('RGB', (self.width,self.height), color='black')
        for player in self.players:
            # Snake head
            image.putpixel(player.snake[0], player.color[0])
            for x, y in player.snake[1:]:
                image.putpixel((x, y), player.color[1])
        image.putpixel(self.apple, (255, 0, 0))  # apple
        return image

class Snake:
    def __init__(self, wled, players):
        self.wled = wled
        self.players = players
    
    def run(self):
        game = SnakeGame(self.wled.columns, self.wled.rows, self.players)

        while True:
            time.sleep(0.001)
            if game.process_tick():
                self.wled.send_image(game.get_image())
                #TODO update console
            if game.state != 'playing':
                break

        if game.state == 'finished':
            endscreen = Image.open(os.path.join(get_resource_dir(), 'won.png'))
        else:
            endscreen = Image.open(os.path.join(get_resource_dir(), 'gameover.png'))
        
        for _ in range(10):
            self.wled.send_image(endscreen.resize((self.wled.columns, self.wled.rows)))
            time.sleep(0.2)
        game.reset()

def plaympsnake():
    args = get_parsed_args()
    wled = WledSend((args.host, args.port), args.columns, args.rows, args.leds_per_pixel)
    input = KeyboardLinuxV2()
    snake = Snake(wled, ({"name": "Rutger", "input": input.get_dir_p1}, {"name": "Robert", "input": input.get_dir_p2}))
    while True:
        snake.run()

if __name__ == "__main__":
    plaympsnake()
