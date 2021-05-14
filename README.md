build [status](https://travis-ci.com/github/astrosmash/translator/builds/): master ![master](https://travis-ci.com/astrosmash/translator.svg?branch=master), dev ![dev](https://travis-ci.com/astrosmash/translator.svg?branch=dev)

## What is this?
This software is intended to alleviate preparation for a language test, such as IELTS, or facilitate common English knowledge development.

## Main concept
Main idea is to use your own Saved translations as a dictionary.
Thus having a dataset of most common / important words just for you, and checking your progress in learning them.

## Usage steps
### Prepare your dataset
* go to Google Translate
* click on a star (Saved Translations)
* export it to Google Sheets by clicking Sheets icon on the right tab
* go to Google Sheets 
* configure Access for exported document to be Anybody who has a link

### First launch
* copy spreadsheet key (the part behind `/d/` and ahead `/edit` in the URL) 
* copy shpreadsheet ID (digits after `gid=`)
* enter these values in the welcome screen 
* hit Enter on either entry - this will sync the Spreadsheet to the local database (location: `<home directory>/.tiny-ielts./db`)

### ...then
* after that, and on subsequent launches, you will see the Main screen, which will display a random word from the database and an entry for a translation
* after two incorrect translation submissions, you will see a hint
* after five incorrect submissions, you will see the right answer and a field for a sentence for you to compose with the original word
* you can hit Next button at any time to pick different word
###### [YouTube](https://www.youtube.com/watch?v=S_Oi8QRuWjU) screen-recorded demo

### To re-initialize or remove application data
* remove `<home directory>/.tiny-ielts` directory

## TODO
* Implement `remove from DB` right click context option
* Implement `about` right click context option
* Implement a browser of saved sentences database with scroll/pagination, sentence removal option and search entry
* Implement menu for statistics of translation & sentence DBs, such as # of entries and file size
