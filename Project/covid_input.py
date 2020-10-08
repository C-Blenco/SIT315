from fuzzywuzzy import fuzz
from fuzzywuzzy import process
# also pip install python-Levenshtein for faster fuzzy
import json

from covid_process import country_str

def match_input(user_args):
    """Match a users country inputs

    For each country a user enters, this function uses fuzzywuzzy to match to an actual country within the countries.json file
    A user is presented with an option of 3 closely matching strings, if none match, it's removed
    
    Returns a list of country codes used for processing
    """

    with open('countries.json') as f:
        # Load contries dict
        countries_dict = json.load(f)
        # Extract countries and country codes
        countries = {country['Country']: country['ISO2'] for country in countries_dict}
        # Used to store correct country codes
        search_codes = []
        matched_countries = []

        for user_input in user_args:
            input_str = user_input
            # Match user input to country
            matches = process.extract(input_str, [country for country in countries], limit=3)

            # If it's a perfect match, add it to the search codes
            if matches[0][1] == 100:
                search_codes.append(countries[matches[0][0]])
                matched_countries.append(matches[0][0])
            # Otherwise, present user with top 3 matches to select
            else:
                print("\nI found similar results to {}:".format(user_input))
                for i, match in enumerate(matches):
                    print("\t{}. {}".format(i+1, match[0]))

                # Answer menu
                while True:
                    answer = input("Select the correct country (1-3) or enter nothing to remove country: ")
                    if not answer:
                        print("\nNo match found, removing from selected countries.")
                        break
                    elif int(answer) >= 1 and int(answer) <= 3:
                        search_codes.append(countries[matches[int(answer)-1][0]])
                        matched_countries.append(matches[int(answer)-1][0])
                        break

        print("Your selected countries: {}".format(country_str(matched_countries)))
        return search_codes