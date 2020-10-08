import pandas as pd
import matplotlib.pyplot as plt
import os

def savefig(country, province="", city=""):
    """Saves and closes the current plot when exporting, handles filename and location"""

    # Determine filename and directory
    if province and city:
        # print("Savefig - prov, city")
        plt.title("{} - {}, {}".format(country, city, province))
        directory = os.path.join("export", country, province)
        filename = "{} - {} - {}".format(country, province, city)
    elif province:
        # print("Savefig - prov")
        plt.title("{} - {}".format(country, province))
        directory = os.path.join("export", country)
        filename = "{} - {}".format(country, province)
    else:
        # print("Savefig - country")
        plt.title("{}".format(country))
        directory = os.path.join("export", country)
        filename = "{}".format(country)

    # Create directory if it doesn't exist
    if not os.path.exists(directory):
        os.makedirs(directory)
    
    # Save plot
    plt.savefig(os.path.join(directory, filename))
    plt.close()

def export_plot(data, countries, export_cities, queue_worker=None):
    """Processes extracted country data and plots

    For each country in data, export_plot will plot based on whether the data for a certain country has
    data per province & per city (if selected)

    If a queue is passed in for faster multiprocess exporting, plot data is added to queue along with
    the country locale (country, province, city)
    """

    # Iterate over each country
    for country in countries:
        # Separate data into dataframe per country
        df_country = data.loc[data['Country'] == country]

        # Determine if data is per province
        provinces = df_country['Province'].unique()

        # If per province, plot per province
        if provinces.any():
            for province in provinces:
                df_prov = df_country.loc[df_country['Province'] == province]
                cities = df_prov['City'].unique()

                # If export_cities is true, export cities instead of provinces
                if cities.any() and export_cities:
                    for city in cities:
                        df_prov_city = df_prov.loc[df_prov['City'] == city]

                        # Enqueue for processes
                        if queue_worker is not None:
                            queue_worker.push((df_prov_city, (country, province, city)))
                        else:
                            df_prov_city.plot(kind='line', x='Date', y='Active Cases')

                            savefig(country, province, city)
                else:
                    df_total_prov = df_prov.groupby('Date')['Active Cases'].sum().reset_index()

                    # Enqueue for processes
                    if queue_worker is not None:
                            queue_worker.push((df_total_prov, (country, province)))
                    else:
                        df_total_prov.plot(kind='line', x='Date', y='Active Cases')

                        savefig(country, province)
        # Otherwise, plot per country
        else:
            # Enqueue for processes
            if queue_worker is not None:
                queue_worker.push((df_country, (country,)))
            else:
                df_country.plot(kind='line', x='Date', y='Active Cases')

                savefig(country)
    
    # Execute queue finish function
    if queue_worker is not None:
        queue_worker.finish()