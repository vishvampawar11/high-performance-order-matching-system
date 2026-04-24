# This script is a market data simulator that generates a binary stream of market orders and sends it over a UDP connection.
# It takes as input the total number of messages to send and the messages per second rate.
# Out of the total messages sent, 70% will be add orders, 20% will be edit orders and 10% will be delete orders
# The simulator can achieve a maximum throughput of ~175,000 messages per second.
import sys
import time
import random
import socket
import struct

TARGET_IP = "127.0.0.1"
TARGET_PORT = 15000

# Binary layout (must match C++ struct)
# uint8  msg_type
# uint8  side
# uint16 _p0
# uint32 seq_num
# uint64 order_id
# char   symbol[8]
# uint64 price
# uint64 qty
# uint8  _p2[24]
MSG_FORMAT = "<BBHIQ8sQQ24s"


def send_msg(sock, msg_type, seq_num, order_id, symbol, side, price, qty):
    data = struct.pack(
        MSG_FORMAT,
        msg_type,               # uint8
        side,                   # uint8
        0,                      # _p0 (padding)
        seq_num,                # seq_num (mapped to _p1 spot)
        order_id,               # uint64
        symbol.ljust(8, b"\x00"),
        price,                  # uint64
        qty,                    # uint64
        b"\x00" * 24            # padding tail
    )

    sock.sendto(data, (TARGET_IP, TARGET_PORT))

    # Print for visibility
    print(f"[{seq_num:05d}] {chr(msg_type)} | ID: {order_id:<5} "
          f"| {symbol.decode().strip():<4} | {chr(side)} "
          f"| Price: {price/100:7.2f} | Qty: {qty}")


def run_simulator(total_messages, mps):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    next_order_id = 1
    seq = 1
    symbol = b"AAPL"

    max_throughput = (mps == -1)
    interval = 1.0 / mps if not max_throughput else 0

    print(f"Sending {total_messages} messages at {mps} msgs/sec...")

    start_time = time.time()

    total_cycles = max(1, total_messages // 100)

    for cycle in range(total_cycles):
        active_orders = []

        # ---------------------
        # 1. Add = 70%
        # ---------------------
        for _ in range(70):
            order_id = next_order_id
            price = random.randint(9600, 9700)
            qty = random.randint(50, 500)
            side = ord('B') if qty % 2 == 0 else ord('S')

            send_msg(sock, ord('A'), seq, order_id, symbol, side, price, qty)

            active_orders.append([order_id, side, price])
            next_order_id += 1
            seq += 1

            if not max_throughput:
                time.sleep(interval)

        # ---------------------
        # 2. Edit = 20%
        # ---------------------
        to_edit = random.sample(active_orders, 20)
        for (order_id, side, old_price) in to_edit:
            new_qty = random.randint(10, 300)
            new_price = random.randint(9650, 9750)

            send_msg(sock, ord('M'), seq, order_id, symbol, side, new_price, new_qty)
            seq += 1

            if not max_throughput:
                time.sleep(interval)

        # ---------------------
        # 3. Delete = 10%
        # ---------------------
        to_delete = random.sample(active_orders, 10)
        for (order_id, side, _) in to_delete:
            send_msg(sock, ord('D'), seq, order_id, symbol, side, 0, 0)
            seq += 1

            if not max_throughput:
                time.sleep(interval)

    end_time = time.time()
    print(f"Finished in {end_time - start_time:.2f} seconds")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python order_simulator.py <total_messages> <mps>")
    else:
        run_simulator(int(sys.argv[1]), int(sys.argv[2]))