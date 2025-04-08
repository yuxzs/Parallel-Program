import requests # type: ignore
from collections import Counter
import time
import jieba # type: ignore
import re
import argparse
import threading
from queue import Queue
from multiprocessing import Process, Lock, Manager, Pool, cpu_count
# import multiprocessing # multiprocessing.Queue
import asyncio
import aiohttp

parser = argparse.ArgumentParser(description='Final Project')

parser.add_argument('--input_list', type = str, required=True, help = 'Urls file path')

args = parser.parse_args()

# 清理文字內容，去除 HTML、符號，只保留中文
def clean_text(text):
    text = re.sub(r'<[^>]+>', '', text)  # 移除 HTML 標籤
    text = re.sub(r'[^\u4e00-\u9fff]', '', text)  # 只保留中文
    return text

def fetch_and_count(urls):
    total_counter = Counter()
    # headers = {
    #     "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0 Safari/537.36"
    # }
    for url in urls:
        try:
            response = requests.get(url, timeout=300)
            if response.status_code == 200:
                text = clean_text(response.text)
                words = jieba.lcut(text)
                words = [w for w in words if len(w) >= 2]  # 去除一字詞
                total_counter.update(words)
            else:
                print(f"無法存取 {url}，狀態碼：{response.status_code}")
        except requests.RequestException as e:
            print(f"錯誤存取 {url}：{e}")
    return total_counter

# 單一網站爬蟲線程任務
def crawl_and_count(url, queue):
    try:
        response = requests.get(url, timeout=10)
        if response.status_code == 200:
            text = clean_text(response.text)
            words = jieba.lcut(text)
            words = [w for w in words if len(w) >= 2]
            counter = Counter(words)
            queue.put(counter)
        else:
            print(f"無法存取 {url}，狀態碼：{response.status_code}")
    except requests.RequestException as e:
        print(f"錯誤存取 {url}：{e}")

# def multiprocessing_crawl_and_count(url, queue, lock):
def multiprocessing_crawl_and_count(url):
    jieba.setLogLevel(jieba.logging.WARN)
    try:
        response = requests.get(url, timeout=10)
        if response.status_code == 200:
            text = clean_text(response.text)
            # lock.acquire()
            words = jieba.lcut(text)
            # lock.release()
            words = [w for w in words if len(w) >= 2]
            # print(words)
            
            return Counter(words)
            # counter = Counter(words)
            # queue.put(counter)
        else:
            print(f"無法存取 {url}，狀態碼：{response.status_code}")
    except requests.RequestException as e:
        print(f"錯誤存取 {url}：{e}")
    return Counter()
    # print('thread finish')

def extract_words(text):
    text = re.sub(r'<[^>]+>', '', text)
    text = re.sub(r'[^\u4e00-\u9fff]', '', text)
    words = jieba.lcut(text)
    words = [w for w in words if len(w) >= 2]
    return Counter(words)
async def fetch(session, url, sem):
    async with sem:  # 控制同時最多幾個連線
        try:
            async with session.get(url, timeout=10) as response:
                if response.status == 200:
                    html = await response.text()
                    return extract_words(html)
                else:
                    print(f"無法存取 {url}，狀態碼：{response.status}")
        except Exception as e:
            print(f"錯誤存取 {url}：{e}")
    return Counter()

def main(urls):    
    start_time = time.time()
    word_counter = fetch_and_count(urls)
    end_time = time.time()
    print(f"\nSequential 總執行時間：{end_time - start_time:.2f} 秒")

    start_time = time.time()
    queue = Queue()
    threads = []

    # 建立與啟動線程
    for url in urls:
        t = threading.Thread(target=crawl_and_count, args=(url, queue))
        t.start()
        threads.append(t)

    # 等待所有線程完成
    for t in threads:
        t.join()

    # 合併詞頻結果
    total_counter_threading = Counter()
    while not queue.empty():
        total_counter_threading.update(queue.get())

    end_time = time.time()
    print(f"\nThreading 總執行時間：{end_time - start_time:.2f} 秒")

    start_time = time.time()
    # manager = Manager()
    # queue = manager.Queue() # thread safe version
    # # queue = multiprocessing.Queue() threrad unsafe version
    # lock = Lock()
    # processes = []

    pool_size = min(len(urls), cpu_count())  # 使用最適合的核心數

    with Pool(pool_size) as pool:
        results = pool.map(multiprocessing_crawl_and_count, urls)  # 平行處理所有網址

    # print('create')
    # 建立與啟動進程
    # for url in urls:
    #     p = Process(target=multiprocessing_crawl_and_count, args=(url, queue, lock))
    #     p.start()
    #     processes.append(p)

    # print('join')
    # 等待所有進程完成
    # for p in processes:
    #     p.join()

    # 合併所有子進程回傳的 Counter
    # print('finish')
    total_counter_multiprocessing = Counter()
    # while queue.empty():
    #     out = queue.get()
    #     print(dict(out.most_common(20)))
    #     total_counter_multiprocessing.update(out)

    for counter in results:
        total_counter_multiprocessing.update(counter)
    
    end_time = time.time()
    print(f"\nmultiprocessing 總執行時間：{end_time - start_time:.2f} 秒")

    # print("前20個常見詞彙：")
    # for word, freq in total_counter_multiprocessing.most_common(20):
    #     print(f"{word}: {freq}")
    
    return dict(word_counter.most_common(20)), dict(total_counter_threading.most_common(20)), dict(total_counter_multiprocessing.most_common(20))

async def async_main(urls):
    start_time = time.time()
    sem = asyncio.Semaphore(5)  # 最多同時5個請求
    total_counter = Counter()

    async with aiohttp.ClientSession() as session:
        tasks = [fetch(session, url, sem) for url in urls]
        results = await asyncio.gather(*tasks)

        for counter in results:
            total_counter.update(counter)

    end_time = time.time()
    print(f"\nAsyncIO 總執行時間：{end_time - start_time:.2f} 秒")
    return dict(total_counter.most_common(20))

    # print("前20個常見詞彙：")
    # for word, freq in total_counter.most_common(20):
    #     print(f"{word}: {freq}")

if __name__ == "__main__":
    # jieba.initialize()
    jieba.setLogLevel(jieba.logging.WARN)
    urls = list()
    with open(args.input_list, 'r', encoding='utf-8') as f:
        for row in f.readlines():
            urls.append(row.replace('\n',''))
    
    total_Sequential, total_Threading, total_multiprocessing = main(urls)
    total_asyncio = asyncio.run(async_main(urls))

    for key in total_Sequential.keys():
        if total_Sequential.get(key, 0) == total_Threading.get(key, 0) and total_Threading.get(key, 0) == total_multiprocessing.get(key, 0) and total_multiprocessing.get(key, 0) == total_asyncio.get(key, 0):
            pass
        else:
            print(f'{key}: \n\tSequential:{total_Sequential.get(key, 0)}\n\tThreading:{total_Threading.get(key, 0)}\n\tMultiprocessing:{total_multiprocessing.get(key, 0)}\n\tAsyncio:{total_asyncio.get(key, 0)}')