import pygame, sys
import asyncio

from bleak import BleakClient
from button import Button

pygame.init()

SCREEN = pygame.display.set_mode((1280, 720))
pygame.display.set_caption("Autonomous Tractor Controller")

address = "94:a9:a8:3b:14:2f"
MODEL_NBR_UUID = "D8773AC5-48E4-1CA0-E332-0723C91631DE"

BG = pygame.image.load("assets/Background.png")


def notification_handler(sender, data):
    """Simple notification handler which prints the data received."""
    data1 = list(data)
    data2 = ((data1[1]+data1[2]*256)*0.005)
    print("{0}: {1}".format(sender, data2))
    
def get_font(size): # Returns Press-Start-2P in the desired size
    return pygame.font.Font("assets/font.ttf", size)

async def start(address):
    async with BleakClient(address) as client:
            model_number = await client.read_gatt_char(MODEL_NBR_UUID)
            await client.start_notify(MODEL_NBR_UUID, notification_handler)
            print("Writing command")
            await client.write_gatt_descriptor(0x5, bytearray('on\r', 'utf8'))
            print("robot shouldve started")
    while True:
        
        PLAY_MOUSE_POS = pygame.mouse.get_pos()

        SCREEN.fill("black")

        PLAY_TEXT = get_font(25).render("Robot has been started.", True, "White")
        PLAY_RECT = PLAY_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(PLAY_TEXT, PLAY_RECT)

        STOP = Button(image=None, pos=(840, 460), 
                            text_input="STOP", font=get_font(75), base_color="White", hovering_color="Green")
        batteryhealth = 60
        STOP.changeColor(PLAY_MOUSE_POS)
        STOP.update(SCREEN)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if STOP.checkForInput(PLAY_MOUSE_POS):
                    menu()

        pygame.display.update()
    
def options():
    while True:
        OPTIONS_MOUSE_POS = pygame.mouse.get_pos()

        SCREEN.fill("white")

        OPTIONS_TEXT = get_font(45).render("This is the OPTIONS screen.", True, "Black")
        OPTIONS_RECT = OPTIONS_TEXT.get_rect(center=(640, 260))
        SCREEN.blit(OPTIONS_TEXT, OPTIONS_RECT)

        OPTIONS_BACK = Button(image=None, pos=(640, 460), 
                            text_input="BACK", font=get_font(75), base_color="Black", hovering_color="Green")

        OPTIONS_BACK.changeColor(OPTIONS_MOUSE_POS)
        OPTIONS_BACK.update(SCREEN)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if OPTIONS_BACK.checkForInput(OPTIONS_MOUSE_POS):
                    menu()

        pygame.display.update()

def menu():
    while True:
        SCREEN.blit(BG, (0, 0))

        MENU_MOUSE_POS = pygame.mouse.get_pos()

        MENU_TEXT = get_font(100).render("AT Controller", True, "#b68f40")
        MENU_RECT = MENU_TEXT.get_rect(center=(640, 100))

        
        START_BUTTON = Button(image=pygame.image.load("assets/Play Rect.png"), pos=(640, 250), 
                            text_input="Start", font=get_font(75), base_color="#d7fcd4", hovering_color="White")
        TRIP_BUTTON = Button(image=pygame.image.load("assets/Options Rect.png"), pos=(640, 400), 
                            text_input="Recent Trip Report", font=get_font(50), base_color="#d7fcd4", hovering_color="White")
        QUIT_BUTTON = Button(image=pygame.image.load("assets/Quit Rect.png"), pos=(640, 550), 
                             text_input="QUIT", font=get_font(75), base_color="#d7fcd4", hovering_color="White")

        SCREEN.blit(MENU_TEXT, MENU_RECT)

        for button in [START_BUTTON, TRIP_BUTTON, QUIT_BUTTON]:
            button.changeColor(MENU_MOUSE_POS)
            button.update(SCREEN)
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if START_BUTTON.checkForInput(MENU_MOUSE_POS):
                    start()
                if TRIP_BUTTON.checkForInput(MENU_MOUSE_POS):
                    trip()
                if QUIT_BUTTON.checkForInput(MENU_MOUSE_POS):
                    pygame.quit()
                    sys.exit()

        pygame.display.update()

menu()