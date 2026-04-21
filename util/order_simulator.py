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

# message format:
# - msg type    (c)
# - seq num     (I)
# - order id    (Q)
# - symbol      (4s)
# - side        (c)
# - price       (Q)
# - qty         (I)

MSG_FORMAT = "<cIQ4scQI" 

def send_msg(sock, msg_type, seq_num, order_id, symbol, side, price, qty):
    data = struct.pack(
        MSG_FORMAT, 
        msg_type, 
        seq_num, 
        order_id, 
        symbol, 
        side, 
        price, 
        qty
    )

    sock.sendto(data, (TARGET_IP, TARGET_PORT))

    m_type = msg_type.decode()
    s_side = side.decode()
    s_sym = symbol.decode().strip()
    
    print(f"[{seq_num:05d}] {m_type} | ID: {order_id:<4} | {s_sym:<4} | {s_side} | Price: {price/100:>7.2f} | Qty: {qty}")

def run_simulator(total_messages, mps):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    next_id = 1
    seq = 1
    symbol = b"AAPL"

    max_throughput = (mps == -1)
    
    interval = 1.0 / mps
    
    print(f"Sending {total_messages} messages at {mps} msgs/sec...")

    start_time = time.time()

    total_cycles = total_messages // 100

    for cycle in range(total_cycles):
        active_orders = []
        
        # 1. Add 70 orders

        for _ in range(70):
            order_id = next_id
            price = random.randint(9600, 9700) # price in cents
            qty = random.randint(50, 500)
            side = b'B' if qty % 2 == 0 else b'S'

            send_msg(sock, b'A', seq, order_id, symbol, side, price, qty)

            active_orders.append([order_id, side, price])
            next_id += 1
            seq += 1
        
            if not max_throughput:
                time.sleep(interval)

        '''
        # 2. Edit 20 orders
        
        to_edit = random.sample(active_orders, 20)
        for [order_id, side, price] in to_edit:
            new_qty = random.randint(10, 50)
            new_price = random.randint(9650, 9700) if new_qty % 2 == 0 else price 
            send_msg(sock, b'M', seq, order_id, symbol, side, new_price, new_qty)
            seq += 1

            if not max_throughput:
                time.sleep(interval)            
        '''

        # 3. Delete 10 orders

        to_delete = random.sample(active_orders, 10)
        for [order_id, side, price] in to_delete:
            send_msg(sock, b'D', seq, order_id, symbol, side, 0, 0)
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
