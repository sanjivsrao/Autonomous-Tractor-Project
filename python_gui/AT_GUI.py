import threading
import asyncio
import pygame
import time
from bleak import BleakClient

async def connectBLE():
    await client.connect()

ADDRESS = "94:A9:A8:3B:14:2F"
CHARACTERISTIC_UUID = "0000FFE1-0000-1000-8000-00805F9B34FB"
client = BleakClient(ADDRESS)
asyncio.run(connectBLE())

pygame.init()
width, height = 800, 600
screen = pygame.display.set_mode((width, height))

pygame.display.set_caption("Bluetooth Control")

# Define button dimensions and positions
button_width = 100
button_height = 40
start_button_x = 250
start_button_y = 290
stop_button_x = 450
stop_button_y = 290

trip_button_x = 350
trip_button_y = 200

# Define button texts
start_text = "Start"
stop_text = "Stop"


def draw_button(x, y, text):
    font = pygame.font.SysFont(None, 24)
    text_surface = font.render(text, True, (246, 186, 111))
    text_rect = text_surface.get_rect()
    text_rect.center = (x, y)
    pygame.draw.rect(screen, (109, 169, 228), (x - button_width//2, y - button_height//2, button_width, button_height))
    screen.blit(text_surface, text_rect)

def draw_time(t):
    font = pygame.font.SysFont(None, 24)
    text = font.render(f"Elapsed Time: {round(t,3)} seconds", True, (246, 186, 111))
    screen.blit(text, (10, height * .8))
    pygame.display.update()
def draw_report():
    font = pygame.font.SysFont(None, 24)
    trip_surface = font.render(f"Trip Report:", True, (246, 186, 111))
    trip_rect = trip_surface.get_rect()
    trip_rect.topleft = (0, height * .75)
    pygame.draw.rect(screen, (109, 169, 228), (0, height * .75, width//2, height))
    screen.blit(trip_surface, trip_rect) 
# def start_ble():
#     loop = asyncio.new_event_loop()
#     asyncio.set_event_loop(loop)
#     loop.run_until_complete(send_on_command())
#     loop.stop()

# def stop_ble():
#     loop = asyncio.new_event_loop()
#     asyncio.set_event_loop(loop)
#     loop.run_until_complete(send_off_command())
#     loop.stop()
async def disconnectBLE():
    await client.disconnect()

async def send_on_command():
    await client.write_gatt_char(CHARACTERISTIC_UUID, bytearray('on', 'unicode_escape'))

async def send_off_command():
    await client.write_gatt_char(CHARACTERISTIC_UUID, bytearray('off', 'unicode_escape'))
        # await asyncio.sleep(1)
        # await client.stop_notify(CHARACTERISTIC_UUID)
def callback(handle, data):
    print(data)
    str = ""
    listData = list(data)
    for x in range(len(listData)):
        str += chr(listData[x])
    print(str.split())
    font = pygame.font.SysFont(None, 24)
    text = font.render(f"Black Tapes Seen: {str.split()[0][0]}", True, (246, 186, 111))
    screen.blit(text, (10, height * .95))
    pygame.display.update()

async def get_data():
    await client.start_notify(CHARACTERISTIC_UUID, callback)
    await asyncio.sleep(5)
    await client.stop_notify(CHARACTERISTIC_UUID)     




running = True
ble_thread = None
screen.fill((255, 235, 235))
draw_button(start_button_x, start_button_y, start_text)
draw_button(stop_button_x, stop_button_y, stop_text)
draw_button(trip_button_x, trip_button_y, "Generate Trip Report")
draw_report()

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:
                if pygame.mouse.get_pos()[0] > start_button_x - button_width//2 and pygame.mouse.get_pos()[0] < start_button_x + button_width//2 and pygame.mouse.get_pos()[1] > start_button_y - button_height//2 and pygame.mouse.get_pos()[1] < start_button_y + button_height//2:
                    # Start new thread to run BleakClient event loop
                    start = time.time()
                    asyncio.run(send_on_command())
                elif pygame.mouse.get_pos()[0] > stop_button_x - button_width//2 and pygame.mouse.get_pos()[0] < stop_button_x + button_width//2 and pygame.mouse.get_pos()[1] > stop_button_y - button_height//2 and pygame.mouse.get_pos()[1] < stop_button_y + button_height//2:
                    # Stop BleakClient event loop
                    asyncio.run(send_off_command())
                    asyncio.run(get_data())
                    end = time.time()
                    elapsedTime = end - start
                    draw_time(elapsedTime)
                elif pygame.mouse.get_pos()[0] > trip_button_x - button_width//2 and pygame.mouse.get_pos()[0] < trip_button_x + button_width//2 and pygame.mouse.get_pos()[1] > trip_button_y - button_height//2 and pygame.mouse.get_pos()[1] < trip_button_y + button_height//2:
                    print("d")
    pygame.display.flip()

    # Check if BleakClient event loop has stopped and join the thread
    if ble_thread and not ble_thread.is_alive():
        ble_thread.join()
        ble_thread = None
pygame.quit()