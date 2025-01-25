# Installatie handleiding
Deze handleiding zal stapsgewijs uitleggen hoe u de klok kunt installeren en gebruiken. Het laatste hoofdstuk zal veel gestelde vragen beantwoorden.
## Installatie 
De klok haalt zijn tijd op vanaf het internet, Dit kan de klok alleen doen zodra de klok verbind met uw WiFi thuis. Nadat de klok met uw WiFi is verbonden zal de klok altijd de goede tijd weergeven.
### Stap 1:
- Sluit de USB aan op een USB adapter van minimaal 1A, 
- Sluit de andere kant aan op de controller van de klok

<img src="images/handleiding/Aansluiting_USB.jpg" alt="drawing" width="160"/>
<img src="images/handleiding/Aansluiting_2.jpg" alt="drawing" width="160"/>
<img src="images/handleiding/Aansluiting_3.jpg" alt="drawing" width="160"/>

- Uw klok zal opstarten en de onderstaande tekst zal over het scherm scrollen\
 `No WiFi Connect to woordklok WiFi and sign in`

### Stap 2 
- Open op uw telefoon/Laptop/tablet de WiFi instellingen
- Zoek naar de Wifi met de naam `woordklokxxxxxx` (de x maakt uw klok uniek) \
<img src="images/handleiding/Woordklok_WiFi.png" alt="drawing" width="160"/>
- Verbind met de `woordklok` WiFi
- Klik op de Pop-up om in te loggen(de meeste telefoons doen dit automatisch):\
<img src="images/handleiding/sign_in_popup.png" alt="drawing" width="160"/>
- Als alles goed is gegaan ziet u nu deze pagina: \
<img src="images/handleiding/Inlog_pagina.png" alt="drawing" width="160"/>
- Indien u de Pop-up niet krijgt zoals bij sommige Samsung telefoons. 
- drukt u lang op de woordklokxxxxxx wifi en daarna op router beheer zoals: \
<img src="images/handleiding/router_beheer_changes.jpeg" alt="drawing" width="160"/>

### Stap 3
- Selecteer uw WiFi Thuis door op de `Select` te klikken
- Voer het wachtwoord in van uw Thuis netwerk
- klik op Save om de instelling te bevestigen
- De klok zal herstarten en verbinding maken met uw Thuisnetwerk

## Gebruik
De Klok is nu verbonden met uw Thuis netwerk en kan de tijd van het internet halen. Indien u de kleuren wil aanpassen of het gedrag van de klok dan kunt u de onderstaande stappen volgen.
### Stap 1
- Weet u het IP adres al? ga naar Stap 2
- Herstart de klok door de voeding 5 seconde los te halen en weer aan te sluiten
- Neem wat afstand van de klok om de cijfers goed te kunnen lezen
- Schrijf de cijfers op die langs gelopen komen: \
- Voorbeeld: 192.168.178.13 of 192.168.2.84 (meestal is alleen het laatste getal anders)
- Op deze foto ziet u het getal 168 waarvan alleen 16 momenteel in beeld is \
<img src="images/handleiding/ip_address.jpg" alt="drawing" width="160"/>

### Stap 2
- Pak u telefoon/laptop/tablet
- Controller dat u verbonden bent met uw Thuisnetwerk
- Open een browser (chrome, safarie, Edge, ..)
- Voer het IP adres in de adres balk en open de site \
<img src="images/handleiding/URL_browser.png" alt="drawing" width="200"/>

### Stap 3
De website van de klok zal verschijnen, Linksboven kan het menu opengeklapt worden
<img src="images/handleiding/website_menu.png" alt="drawing" width="200"/>
- Clock Config
    - De kleur is de kleur van de tijd tekst
    - Tekst effect is het effect op de tijd (Uit, Random of Rainbow)
    - Helderheid van 0 tot 5 ingesteld, dit is een extra offset in de helderheid. (5 is het felst)
    - achtergrondkleur, is de kleur van alle letter die niet de tijd tekst weergeven.
    - Animaties, is een keuze van een aantal animaties tussen elke minuut overgang
    - Tekst, Is een aanpasbare tekst die als loopkrant langs kan komen.
    - Save, hiermee wordt de nieuwe instelling actief

    <img src="images/handleiding/website_clock_config.png" alt="drawing" width="200"/>
- Wifi config
    - SSID is de naam van de wifi netwerk waarmee de klok moet verbinden.
    - Password, het wachtwoord van het wifi netwerk

    <img src="images/handleiding/website_wifi_config.png" alt="drawing" width="200"/>
- Systeem Config
    - Hardware versie, 13x13 of 11x11 is het aantal letters in de lengte en breedte.
        - 11x11 is voor de klein of grote klok met 11x11 letters
        - 13x13 is voor letterplaat versie 1 50x50cm
        - 13x13 not Accurate is voor versie 1, waarbij de text Bijna en geweest gebruikt worde
        - 13x13 V2 is voor letterplaat versie 2, waarbij 16:46 -> Het is negentien voor vijf
        - 13x13 V2.1 is voor letterplaat versie 2, waarbij 16:46 -> Het is elf over half vijf.
    - Pixel type, is het type ledstrip
        - RGB is standaard voor kleine en oude klokken
        - RGBNW is standaard voor nieuwe groten klokken
    - Perfect inperfections, hiermee wordt er soms een willekeurige led aangezet
    - Hier ben ik, is een functie die lastig te gebruiken is. 
    - OTA fw server, Dit is de server waar de klok nieuwe FW kan ophalen
    
    <img src="images/handleiding/website_systeem_config.png" alt="drawing" width="200"/>

## Veel gestelde vragen
### Hoe vind ik de webpagina van de klok?
Herstart de klok, en lees het IP adres af vanaf een afstand, dit adres kunt u invoeren in een browser en de webpagina zal verschijnen

### Er brand altijd een rode letter in de hoek?
Uw klok kan momenteel geen verbinding maken met het internet, Probeer de klok te herstarten indien daarmee het probleem niet is verhopen is misschien uw WiFi veranderd volg de installatie stappen

### Hoe pas ik de kleur van de letter aan?
De kleur van de klok kan aangepast worden via de website. De website is via het IP adres van de klok beschikbaar

### Ik kan het IP adres niet lezen?
Neem wat afstand van de klok zodat u duidelijker cijfers zult zien.
Het IP adres komt langs als loopkrant text.

### Mijn Thuis Wifi staat niet in de lijst en nu?
Door op de `refresh` knop te klikken zal de klok opnieuw opzoek gaan naar WiFi netwerken in de buurt.
De klok verijst een 2.4GhZ WiFi netwerk, 5Ghz wordt niet ondersteund, vaak kunt u dit instellen op uw thuis netwerk.

### Mijn klok doet gek, de letters staan niet goed of de woorden kloppen niet?
Gezien de klok in twee maten verkocht worden kan het zijn dat de systeem instellingen van de klok niet meer goed staan, Dit kun u herstellen door:
- Onder de "system config" controlleer de "hardware versie"
    - Als uw klok 11 rijen heeft en 11 kolomen dan moet het 11x11 zijn
    - Als uw klok 13 rijen heeft en 13 kolomen dan moet het 13x13 zijn als er "viertien" op de plaat staat
    - Als uw klok 13 rijen heeft en 13 kolomen dan moet het 13x13 V2 zijn als er "veertien" op de plaat staat
- Onder de "system config" controlleer de "pixel type"
    - Al het onderste deel van de klok niet verlicht is heeft u waarschijnlijk RGBNW
    - Anders kunt u vragen welke variant u heeft
- Na elke wijziging moet u op save knop drukken 2 seconde wachten en daarna op de reboot knop.

### Firmeware update
U kunt de firmware van de klok updaten door de onderstaande stappen te doen.
- Ga via uw telefoon of laptop naar het ip address van de klok
- Daarna naar "Systeem config"
- Schrijf de "build date" op (staat onderin de pagina)
- Pas het "OTA fw server" veld aan naar "http://download.wssns.nl" "8090" "Release"
- Druk op "Save" knop, en wacht 2 seconde
- Druk op "sw update" knop
- Uw klok herstart, controlleer of het gelukt is door naar "Systeem config" te gaan en de build date te vergelijken.

### De fabrieks instellingen terug zetten
De klok terug zetten naar fabrieksinstellingen:
- Ga via uw telefoon of laptop naar het ip address van de klok
- Daarna naar "Systeem config"
- Druk op "Factory Reset" knop

### Mijn klok is kapot en nu?
Neem contact op zolang u vriendelijk blijft is er een hoop mogelijk.

