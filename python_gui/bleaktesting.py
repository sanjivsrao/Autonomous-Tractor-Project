import asyncio
from bleak import BleakClient


address = "94:A9:A8:3B:14:2F"
c_UUID = "0000FFE1-0000-1000-8000-00805F9B34FB"
s_UUID = "0000FFE0-0000-1000-8000-00805F9B34FB"
def callback(handle, data):
  print(handle, data)

async def main(address):
    async with BleakClient(address) as client:
        await client.write_gatt_char(c_UUID, bytearray('off', 'unicode_escape'))
        
        await client.start_notify(s_UUID, callback) 
        #model_number = await client.write_gatt_char(c_UUID)
        #print("Model Number: {0}".format("".join(map(chr, model_number))))

asyncio.run(main(address))