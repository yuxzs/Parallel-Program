import requests
from collections import Counter
import time
import jieba
import re
import argparse
import threading
from queue import Queue

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

def main():
    urls = list()
    with open(args.input_list, 'r', encoding='utf-8') as f:
        for row in f.readlines():
            urls.append(row.replace('\n',''))
    
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
    total_counter = Counter()
    while not queue.empty():
        total_counter.update(queue.get())

    end_time = time.time()
    print(f"\nThreading 總執行時間：{end_time - start_time:.2f} 秒")


    print("前20個常見詞彙：")
    for word, freq in total_counter.most_common(20):
        print(f"{word}: {freq}")

if __name__ == "__main__":
    main()