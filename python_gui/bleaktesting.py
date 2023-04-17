

def callback(handle, data):
  print(data)
  res = ''.join(map(chr, data))
  print(str(res))

async def main(address):
    async with BleakClient(address) as client:
        print(client.is_connected)
        await client.write_gatt_char(c_UUID, bytearray('off', 'unicode_escape'))
        
        await client.start_notify(c_UUID, callback) 
        await asyncio.sleep(5)
        await client.stop_notify(c_UUID)
        #model_number = await client.write_gatt_char(c_UUID)
        #print("Model Number: {0}".format("".join(map(chr, model_number))))
        await client.disconnect()

asyncio.run(main(address))

import pygame, sys
import asyncio

from bleak import BleakClient
from button import Button

pygame.init()
avgSpeed = 0
totalSpeed = 0
blacklines = 0
speedI = 0


SCREEN = pygame.display.set_mode((1280, 720))
pygame.display.set_caption("Autonomous Tractor Controller")

address = "94:A9:A8:3B:14:2F"
c_UUID = "0000FFE1-0000-1000-8000-00805F9B34FB"
s_UUID = "0000FFE0-0000-1000-8000-00805F9B34FB"
BG = pygame.image.load("assets/Background.png")


def notification_handler(sender, data):
    """Simple notification handler which prints the data received."""
    data1 = list(data)
    data2 = ((data1[1]+data1[2]*256)*0.005)
    print("{0}: {1}".format(sender, data2))
    
def get_font(size): # Returns Press-Start-2P in the desired size
    return pygame.font.Font("assets/font.ttf", size)

def start():
    startBot(address)
    speed = 0
    while True:
        
        PLAY_MOUSE_POS = pygame.mouse.get_pos()

        SCREEN.fill("black")
        data_text= get_font(25).render("Real-Time Data", True, "Orange")
        data_rect = data_text.get_rect(center=(300, 100))
        SCREEN.blit(data_text, data_rect)
        #gets values 
        batteryhealth = updateBattery(address)
        accel = updateAccel(address)
        speed = speed + .001 * accel
        totalSpeed = totalSpeed + speed
        speedI = speedI + 1
        blackLines = updateBlackLines(address)
        direction = updateDirection(address)
        #displays data
        battery_text= get_font(25).render("Battery Health: " + str(batteryhealth) + "%", True, "White")
        battery_rect = data_text.get_rect(center=(300, 150))
        SCREEN.blit(battery_text, battery_rect)
        speed_text = get_font(25).render("Speed: " + str(speed) + "cm/s", True, "White")
        speed_rect = data_text.get_rect(center=(300, 200))
        SCREEN.blit(speed_text, speed_rect)
        blackLines_text = get_font(25).render("Black Lines: " + str(blackLines), True, "White")
        blackLines_rect = data_text.get_rect(center=(300, 250))
        SCREEN.blit(blackLines_text, blackLines_rect)
        direction_text = get_font(25).render("Direction: " + str(direction), True, "White")
        direction_rect = data_text.get_rect(center=(300, 300))
        SCREEN.blit(direction_text, direction_rect)

        STOP = Button(image=None, pos=(840, 460), 
                            text_input="STOP", font=get_font(75), base_color="White", hovering_color="Green")
        STOP.changeColor(PLAY_MOUSE_POS)
        STOP.update(SCREEN)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if STOP.checkForInput(PLAY_MOUSE_POS):
                    stopBot(address)
                    trip()
        pygame.display.update()
    
def trip():
    while True:
        OPTIONS_MOUSE_POS = pygame.mouse.get_pos()

        SCREEN.fill("black")

        OPTIONS_TEXT = get_font(45).render("Trip Info:", True,"White")
        OPTIONS_RECT = OPTIONS_TEXT.get_rect(center=(640, 50))
        SCREEN.blit(OPTIONS_TEXT, OPTIONS_RECT)

        #display overall data
        avgSpeed = totalSpeed / speedI
        avgSpeed_text = get_font(25).render("Average Speed: " + str(avgSpeed) + "cm/s", True, "White")
        avgSpeed_rect = avgSpeed_text.get_rect(center=(640, 150))
        SCREEN.blit(avgSpeed_text, avgSpeed_rect)
        totalBlackLines_text = get_font(25).render("Total Black Lines: " + str(blacklines), True, "White")
        totalBlackLines_rect = totalBlackLines_text.get_rect(center=(640, 200))
        SCREEN.blit(totalBlackLines_text, totalBlackLines_rect)

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
async def startBot(address):
    async with BleakClient(address) as client:
      await client.write_gatt_char(c_UUID, bytearray('on', 'unicode_escape'))
      print("robot shouldve started")
async def stopBot(address):
    async with BleakClient(address) as client:
      await client.write_gatt_char(c_UUID, bytearray('off', 'unicode_escape'))
      print("robot shouldve stopped")
async def updateBattery(address):
    async with BleakClient(address) as client:
        await client.start_notify(s_UIUD, notification_handler)
        await asyncio.sleep(1.0)
        await client.stop_notify(s_UIUD)
        return 50
def updateAccel(address):
#     async with BleakClient(address) as client:
#         await client.start_notify(s_UIUD, notification_handler)
#         await asyncio.sleep(1.0)
#         await client.stop_notify(s_UIUD)
    return 509
async def updateBlackLines(address):
    async with BleakClient(address) as client:
        await client.start_notify(s_UIUD, notification_handler)
        await asyncio.sleep(1.0)
        await client.stop_notify(s_UIUD)
        return 50
async def updateDirection(address):
    async with BleakClient(address) as client:
        await client.start_notify(s_UIUD, notification_handler)
        await asyncio.sleep(1.0)
        await client.stop_notify(s_UIUD)
        return 50
menu()