import pprint
import argparse
import sys
import time
import pandas as pd
from multiprocessing import Pool, Manager, Process
from clint.textui import progress
import requests
import os.path

# custom functions
from covid_plot import savefig, export_plot
from covid_process import process_countries, country_str
from covid_input import match_input

class QueueWorker():
    """ Queue Worker

    The queue worker handles the put/get of queue items for both worker processes and the producer process.
    When finished, it will ensure the "DONE" command is sent to all processes (n_workers)
    """

    def __init__(self, queue, n_workers):
        self.queue = queue
        self.n_workers = n_workers

    def push(self, data):
        self.queue.put(data)

    def get(self):
        return self.queue.get()

    def finish(self):
        for _ in range(self.n_workers):
            self.queue.put(("DONE", None))
 
def savefig_process(queue_worker, process):
    """Function for multiprocessed export: exports plots from data in queue"""
    while True:
        # Get the dataframe and locale (country, province, city) from queue
        data, locale = queue_worker.get()

        # If done command recieved, break loop
        if (type(data) == str):
            if (data == "DONE"):
                # print("Process {} export done.".format(process))
                break
        # else plot data
        else:
            data.plot(kind='line', x='Date', y='Active Cases')
            savefig(*(locale)) # *(locale) to unpack tuple into args

def download_dataset():
    """Function downloads corona dataset if it doesn't exist"""

    path = 'corona.json'
    if not os.path.isfile(path):
        print("Downloading COVID-19 dataset, please wait.")
        url = 'https://api.covid19api.com/all'
        r = requests.get(url, stream=True)

        with open(path, 'wb') as f:
            for chunk in r.iter_content(chunk_size=1024):
                if chunk:
                    f.write(chunk)
                    f.flush()
    else:
        print("COVID-19 Dataset exists.")

def main():
    download_dataset()
    # Argument parser
    parser = argparse.ArgumentParser(description='Extract COVID-19 data for selected countries')
    parser.add_argument(
        '-c', '--countries', nargs='+', type=str, required=True, dest='countries', help='Specify list of countries to process')
    parser.add_argument(
        '-p', '--processes', action="store", type=int, dest="processes", default=1, help='Specify number of processes')
    parser.add_argument(
        '-cities', '--city_export', dest='export_cities', default=False, action='store_true')
    args = parser.parse_args()
    
    # Get corresponing country codes and match country inputs
    country_codes = match_input(args.countries)

    # Process countries
    print("Tasks:")
    print("\t- Begin processing")
    p_start_time = time.time()
    data, countries = process_countries(country_codes)
    p_time_total = time.time() - p_start_time
    print("\t  Processing finished\t{}s.".format(round((p_time_total), 2)))

    # Export plots --
    print("\t- Begin export")
    e_start_time = time.time()
    if (args.processes > 1):
        print("\t  Export method: Multi-process export ({} processes)".format(args.processes))
        # Create multiproccessing manager, queue and queue worker
        mgr = Manager()
        data_queue = mgr.Queue()
        queue_worker = QueueWorker(data_queue, args.processes)

        # Start processes 
        processes = []
        for i in range(args.processes):
            p = Process(target=savefig_process, args=((queue_worker), i + 1))
            processes.append(p)
            p.start()

        # Generate plotting data, adding a queue_worker will add to queue instead of plotting
        export_plot(data, countries, args.export_cities, queue_worker)

        # Join finished processes
        for p in processes:
            p.join()
    else:
        print("\t  Export method: Serial export")
        export_plot(data, countries, args.export_cities)
    e_time_total = time.time() - e_start_time
    print("\t  Export finished\t{}s.".format(round((e_time_total), 2)))

    print("Total execution time {} seconds.".format(round((e_time_total + p_time_total), 2)))


if __name__ == "__main__":
    main()

