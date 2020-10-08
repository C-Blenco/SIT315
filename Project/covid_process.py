import pandas as pd
import ijson
from queue import Queue
import sys

def country_str(countries):
    """Returns a formatted string in the form of "country, country, country"..."""

    string = ", ".join(["{}"]*len(countries)).format(*countries)
    return string

def to_dataframe(data, return_countries=True):
    """Creates dataframe from dictionary object"""

    df = pd.DataFrame(data)
    # String to datetime
    df['Date'] = pd.to_datetime(df['Date'])
    if return_countries:
        countries = df['Country'].unique()
        return df, countries
    else:
        return df

def process_countries(countries):
    """Process data from large corona dataset

    Iteratively processes the selected "countries" (list) from the corona.json dataset
    Returns a dataframe of the extracted data
    """

    f = open('corona.json', 'rb')
    # Iterate over JSON objects
    items = ijson.items(f, 'item')
    success = { i : False for i in countries }

    data = []
    # For each JSON object
    for item in items:
        if item['CountryCode'] in countries:
            data.append({
                'Country': item['Country'], 
                'Province': item['Province'],
                'City': item['City'],
                'Date': item['Date'], 
                'Active Cases': item['Active']
                })
            success[item['CountryCode']] = True

    # If empty, no data was found for supplied countries, so exit
    if not (True in success.values()):
        sys.exit("No data found for all countries provided.")
    # If some data wasn't found for countries, print
    if False in success.values():
        failed = []
        for country, successful in success.items():
            if not successful:
                failed.append(country)
        print("No data found for countries: {}".format(country_str(failed)))

    # Return 'data' dataframe and country series
    return to_dataframe(data)