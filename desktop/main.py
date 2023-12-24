import time
import asyncio

import mouse
from bleak import BleakScanner, BleakClient

service_uuid = "ae088d12-a31e-4d8c-8b2b-84dd292883f4"
char_uuid = "49a380be-0591-4bb0-978b-fab6cc055f3f"
mac_address = 'DC:54:75:D0:88:F5'

mouse_move_evt = asyncio.Event()


# noinspection PyUnusedLocal
def cb(evt):
    mouse_move_evt.set()


mouse.hook(cb)


async def scan():
    print("scanning devices")
    client_mac_addresses = set()
    devices = await BleakScanner.discover(return_adv=True)

    for k, v in devices.values():
        # print(v.local_name, v.service_uuids)
        if service_uuid not in v.service_uuids:
            continue

        if k.address not in client_mac_addresses:
            print("found:", k.name, k.address)
            client_mac_addresses.add(k.address)

    return client_mac_addresses


async def send_heartbeat(address):
    print(f"connecting[{address}]")

    try:
        async with BleakClient(address) as client:
            print(f"connected[{address}]")

            parameter = int(time.time())

            print(f"sending data[{address}]: {parameter}")
            await client.write_gatt_char(char_uuid, f"{parameter}".encode("utf8"))

        print(f"disconnected[{address}]")

    except Exception as err:
        print("exception:", err)


async def main():
    print("Hello world!")
    while True:
        if mouse_move_evt.is_set():
            print("mouse moved")

            async with asyncio.TaskGroup() as tg:
                for addr in await scan():
                    tg.create_task(send_heartbeat(addr))

            print("longer sleep")
            await asyncio.sleep(10 * 60)
            print("sleep done")
            mouse_move_evt.clear()

        await asyncio.sleep(1)


asyncio.run(main())
