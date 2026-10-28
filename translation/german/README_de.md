# CLCL - Clipboard-Manager

**Version:** 2.2.1

## Übersicht
CLCL ist ein leistungsfähiges Programm zur Verwaltung der  Windows Zwischenablage mit Historisierung, Plugin-Unterstützung und frei wählbaren Hotkeys.

Es bietet schnellen Zugriff auf die zuletzt verwendeten Zwischenablagee-Objekte über ein konfigurierbares Popup-Menü und unterstützt verschiedene Formate wie Text, Bilder und Dateien.

## Funktionen
- **Verlaufs-Verwaltung** - Inhalt der Zwischenablage automatisch im Verlauf speichern & bei Bedarf erneut verwenden
- **Vorlagen-Verwaltung** - Häufig verwendete Texte und Objekte zur schnellen Nutzung speichern
- **Unterstützung aller Formate** - Verarbeitet automatisch alle Formate der Zwischenablage
- **Menüanzeige per Hotkey** - Schnelles Popup-Menü des Verlaufs über Alt+C (konfigurierbar)
- **Benutzerdefinierte Menüs** - Vollständig anpassbarer Menüaufbau und -darstellung
- **Bildvorschau** - Miniaturansicht von Bitmap-Inhalten im Menü
- **Tooltip-Hinweise** - Tooltip-Anzeige für Elemente des Zwischenablage-Verlaufs, Vorlagen und Tool-Plugins
- **Dunkler Modus** - Entsprechend Windows-Einstellungen
- **Formatsteuerung** - Selektives Speichern bestimmter Zwischenablageformate
- **Größe des Verlaufs** - Maximale Anzahl der Elemente im Verlauf konfigurierbar
- **Plugin-Architektur** - Erweiterbare Funktionalität durch Plugin-DLLs
- **Fensterfilter** - Zwischenablage bestimmter Anwendungen vom Verlauf ausschließen
- **Benutzerdefinierte Tasten zum Einfügen** - Anwendungs-spezifische Konfiguration der Paste-Taste
- **Unicode-Unterstützung** - Vollständige Unicode-Unterstützung für internationalen Text
- **Mehrsprachig** - UI-Sprache wählbar: Deutsch, Englisch, Ukrainisch, Japanisch, Vereinfachtes Chinesisch
- **Binär-Anzeige** - Rohdaten der Zwischenablage im Hex-Format anzeigen
- **Lokale Speicherung aller Daten** - Keine Übertragung in die Cloud
- **Kostenlos und Open Source** - auf [github.com/nakkag/CLCL](https://github.com/nakkag/CLCL)

__Zusätzliche Funktionen nur in der extended bzw experimentellen Version__
--

- **Schnellsuche** - im Tray-Menü und mit Hotkey
- **Installationsprogramm mit optionalen Plugins** - mit vielen zusätzlichen Funktionen
- **Integrierte Hilfe** - Drücken Sie `<F1>` in Einstellungen oder Viewer für englische oder deutsche Hilfe
- **Kostenlos und Open Source** - Aktive Entwicklung auf [github.com/wilfz/CLCL](https://github.com/wilfz/CLCL)

## Installation
CLCL läuft auf Windows 7 und neuer.

Für die "Nakka pure" Version laden Sie [clcl221.exe](https://nakka.com/soft/clcl/download/clcl221.exe) oder [clcl221.zip](https://nakka.com/soft/clcl/download/clcl221.zip) herunter und installieren Sie diese.

Für die erweiterte Plus-Version laden Sie [setup_clcl_2_2_1.exe](https://www.linguversa.de/clcl/download/setup_clcl_2_2_1.exe) herunter und starten Sie dieses.
Möglicherweise zeigt das Installationsprogramm eine Warnung an. Dies bedeutet nicht automatisch ein Sicherheitsrisiko: als freies Open-Source-Projekt wurde CLCL aber nicht kostenpflichtig signiert.

Zum Deinstallieren gehen Sie zur Systemsteuerung und deinstallieren Sie von dort, **nachdem** Sie CLCL geschlossen haben.

Statt des automatischen Installationsprogramms können Sie auch [clcl_2_2_1.zip](https://www.linguversa.de/clcl/download/clcl_2_2_1.zip) herunterladen, in einen Ordner Ihrer Wahl entpacken und `clcl.exe` von dort aus starten.

### Datenspeicherung
Standardmäßig werden Daten und Einstellungen in diesem Verzeichnis gespeichert (für Windows 10/11):

	C:\Users\{username}\AppData\Local\CLCL

### Portable-Modus
Um Daten und Einstellungen im gleichen Verzeichnis wie `CLCL.exe` zu speichern, setzen Sie `clcl_app.ini` wie folgt und starten CLCL.
```ini
[GENERAL]
portable=1
```


### Spracheinstellungen
Die Menüs und Dialoge sind in Englisch, Japanisch, Deutsch, vereinfachtem Chinesisch oder Ukrainisch entsprechend Ihren Windows-Spracheinstellungen.
Sie können dies überschreiben, indem Sie explizit die Sprache auf der Registerkarte Viewer in Optionen einstellen.
Die ausgewählte Sprache wird im Abschnitt `[main]` von `CLCL.ini` gespeichert.
```ini
[main]
...
language="en"
```

## Verwendung

### Grundlegende Bedienung

**Start:**

Nach dem Start von `CLCL.exe` erscheint ein Büroklammer-Symbol im Tray-Bereich der Taskleiste.

**Menü anzeigen:**
- Klicken Sie mit der linken Maustaste auf das Symbol in der Taskleiste, um das Menü zu öffnen  
  oder
- Drücken Sie Alt+C (Standard-Hotkey), um das Menü anzuzeigen  
- Wählen Sie danach ein Element aus, um es in das aktive Fenster einzufügen
- Klicken Sie mit der rechten Maustaste auf ein Menüelement, um zusätzliche Optionen anzuzeigen

Standardmäßig zeigt das Menü den [Verlauf](#verlauf) in absteigender Reihenfolge an (neueste zuerst).
Das Menü kann in den Einstellungen angepasst werden.

**Viewer-Fenster:**
- Klicken Sie mit der rechten Maustaste auf das Taskleistensymbol, um den Viewer zu öffnen
- Linkes Panel des Viewers: Zeigt Verlauf und Vorlagen in einer Baumansicht
	- Durchsuchen Sie Zwischenablage-Verlauf und Vorlagen
	- Doppelklicken Sie, um ein Element in die Zwischenablage zu kopieren
	- "Zwischenablage" ganz oben im Baum ist der aktuelle Inhalt der Zwischenablage.
	- ["Verlauf"](#verlauf) im Baum ist eine Liste der neuesten Elemente der Zwischenablage.
	- ["Vorlagen"](#vorlagen) im Baum ist eine Liste von Vorlagenobjekten (wie Textbausteine, Standardphrasen, etc.).
- Rechtes Panel: Anzeigen und Bearbeiten des Inhalts des ausgewählten Elements; wenn ein Baumknoten ausgewählt ist, werden alle Unterelemente als Liste mit (dem Anfang von) deren Daten, Änderungszeit, sowie einem ggf. zugeordneten Hotkey angezeigt.  

**Baumstruktur:**

	┌─■ Zwischenablage - Aktueller Inhalt der Zwischenablage
	│ ├─□ TEXT - Formate in der aktuellen Zwischenablage
	│ ├─□ LOCALE
	│ └─□ OEM TEXT
	│
	├─■ Verlauf - Verlauf der Zwischenablage
	│ ├─□ (BITMAP) - Verlaufselemente
	│ │ ├─□ BITMAP - Formate in Verlaufselementen
	│ │ └─□ DIB
	│ │
	│ ├─□ Hello...
	│ │ └─□ TEXT
	│ │
	│ ├─□ Today...
	│ └─□ (BITMAP)
	│
	└─■ Vorlagen - Vorlagenobjekte
	  │
	  ├─■ Ordner - Ordner
	  │ ├─□ Adresse...
	  │ └─□ (BITMAP)
	  │
	  └─□ http://www... - Vorlage
		└─□ TEXT - Format in der Vorlage

### Menü
Wenn Sie die Maus über ein Verlaufs- oder Vorlagenelement im Menü bewegen, wird an der Mausposition ein Tooltip mit detaillierten Informationen angezeigt. Wenn Sie ein Element mit der Tastatur auswählen, wird ein Tooltip unter dem Menüelement angezeigt.

Ein Rechtsklick auf ein [Verlaufs](#verlauf)- oder [Vorlagen](#vorlagen)-Element im Menü zeigt die registrierten Tools in einem neuen Popup-Menü an. Wird hier ein Tool ausgewählt, so wird es auf den Inhalt des Elements angewendet und das Ergebnis in die Zwischenablage kopiert.

Um das Tool-Menü mit der Tastatur anzuzeigen, drücken Sie Strg- und Eingabetaste und wählen dann ein Werkzeug aus.

Die im Taskleistenbereich oder mit Hotkey angezeigten Menüelemente werden in der Optionsseite ["Aktion"](#aktion) konfiguriert.
Sie können das Menüverhalten und die Anzeige der Elemente auf der Registerkarte ["Menü"](#menüeinstellungen) der Einstellungen anpassen.

### Verlauf
Neu kopierte Daten werden oben im Verlauf hinzugefügt.

Jeder einzelne Verlaufseintrag kann mehrere Clipboard-Formate enthalten. 
Verwenden Sie die Option ["Format"](#format) in den Einstellungen, um bevorzugte Formate zu registrieren und die Priorität beim Anzeigen von Elementen zu steuern.  
Das Clipboard-Format mit der höchsten Priorität wird im Menü und Viewer angezeigt.

Die Anzahl der zu speichernden Elemente im Verlauf wird im Reiter ["Verlauf"](#verlaufseinstellungen) der CLCL-Einstellungen festgelegt.

Die Option ["Filter"](#filter) in den Einstellungen bestimmt, welche Formate dem Verlauf hinzugefügt werden.

### Vorlagen
Speichern Sie häufig verwendete Daten wie Standardphrasen, Textschnipsel, URLs, etc. als Vorlagen.
Sie können Vorlagenelemente benennen und sie in Ordnern hierarchisch organisieren.

**Um ein Element hinzuzufügen:**
1. Wählen Sie im Viewer den Inhalt der Zwischenablage oder einen Verlaufseintrag aus
2. Klicken Sie mit der rechten Maustaste, um das Kontextmenü zu öffnen, und wählen Sie "Zu Vorlagen hinzufügen"
3. Wählen Sie optional einen Unterordner aus, in dem das Element gespeichert werden soll.

**Um einen Ordner hinzuzufügen:**
1. Klicken Sie im Viewer mit der rechten Maustaste auf Vorlage oder einen Unterordner davon
2. Wählen Sie "Neuer Ordner" aus dem Kontextmenü

**Um Elemente/Ordner umzubenennen:**
1. Öffnen Sie den Viewer
2. Wählen Sie das Element aus
3. Klicken Sie mit der rechten Maustaste und wählen Sie "Umbenennen"

**Um einem Element einen Hotkey zuzuweisen:**
1. Klicken Sie im Viewer mit der rechten Maustaste auf ein Vorlagenelement
2. Wählen Sie "Hotkey festlegen ..." aus dem Kontextmenü

**Name löschen** löscht den vorherigen Namen und zeigt den Inhalt des Elements als Namen an. Bei längeren Texten wird in der Anzeige nach eingen Zeichen abgeschnitten und ... angefügt.
Wenn Sie ein Element mit "-" benennen, wird es im Menü als Separator angezeigt. Das Format und die Daten im Element werden ignoriert.

Wenn Sie **&** im Namen einfügen, wird das nachfolgende Zeichen als Shortcut-Key im Menü verwendet. Wenn Sie **&** selbst im Menü anzeigen möchten, verwenden Sie **&&**.

Klicken Sie mit der rechten Maustaste auf ein Vorlagenelement, um das Menü anzuzeigen, und wählen Sie "Hotkey-Einstellungen", um dem Vorlagenelement einen Hotkey zuzuweisen. Beim Betätigen dieses Hotkeys wird dann das Vorlagenelement direkt in die Zwischenablage kopiert, ohne das Menü anzuzeigen. Sofern die Checkbox "Einfügen" zusätzlich aktiviert wurde, wird das Vorlagenelement außerdem in das gerade bearbeitete Dokument eingefügt.
Sie können die registrierten Hotkeys in der Listenansicht des Viewers sehen.  

Es gibt keine Begrenzung für die Anzahl der Vorlagenobjekte oder des Clipboard-Formats.

### In die Zwischenablage senden
Es gibt mehrere Möglichkeiten, Verlauf- oder Vorlagenobjekte in die Zwischenablage zu senden.

- Klicken Sie auf den Taskleistenbereich, um ein Menü anzuzeigen.
Wenn Sie ein Verlauf- oder Vorlagenobjekt aus dem Menü auswählen, werden die Daten in die Zwischenablage kopiert und automatisch in das aktive Fenster eingefügt.

- Drücken Sie den Hotkey (standardmäßig Alt + C), um ein Menü anzuzeigen.
Wenn Sie ein Verlauf- oder Vorlagenobjekt aus dem Menü auswählen, werden die Daten in die Zwischenablage kopiert und automatisch in das aktive Fenster eingefügt.

- Wählen Sie ein Element im Viewer aus und öffnen Sie ein Kontextmenü mit der rechten Maustaste.
Wenn Sie "In die Zwischenablage senden" auswählen, wird das ausgewählte Element in die Zwischenablage kopiert.

## Zwischenablage

### Was ist die Zwischenablage?
Die Zwischenablage ist ein Bereich, in dem verschiedene Anwendungen Daten austauschen können.
Wenn Sie beispielsweise Text in Notepad kopieren und in Word einfügen, verwenden beide Anwendungen die Zwischenablage.

### Zwischenablageformate:
Die Zwischenablage kann mehrere Formate gleichzeitig halten.
Wenn Sie beispielsweise Text in Notepad kopieren, werden die folgenden vier Formate in der Zwischenablage gespeichert (Windows 10/11):

	・UNICODE TEXT
	・LOCALE
	・TEXT
	・OEM TEXT

Wenn Sie in Excel oder Access kopieren, werden noch weitere Formate in die Zwischenablage übertragen.

Standardmäßig ist CLCL so konfiguriert, die folgenden Formate im Verlauf zu speichern:
- UNICODE TEXT (Text)
- BITMAP (Bilder)
- DROP FILE LIST (Dateien)

Verwenden Sie die Option ["Filter"](#filter) in den Einstellungen, um festzulegen, welche Formate gespeichert werden sollen.

Während der Viewer Textdaten und viele Bildformate automatisch anzeigt, werden andere Formate als Binärdaten im Hexdump-Format angezeigt.

Für einige Formate gibt es ein Format-Plugin, um die Zwischenablagedaten benutzerfreundlicher anzuzeigen.  
CLCL wird mit einem RichTextFormat-Plugin geliefert, das auf der Registerkarte ["Format"](#format) der Einstellungen aktiviert werden kann.

## Tools (Plugins)

Tools ermöglichen es Ihnen, die aktuelle Auswahl, den Verlauf oder die Vorlagendaten vor dem Einfügen zu verarbeiten oder die Funktionalität von CLCL zu erweitern. 

Das Installationspaket umfasst Binärdateien einiger nützlicher Plugins von https://nakka.com/soft/clcl/index_eng.html. Diese Plugins wurden so angepasst, dass sie mit aktuellen Betriebssystemen kompatibel sind; insbesondere werden die Plugin-DLLs in denselben Ordner wie `clcl.exe` installiert, die ini-Dateien der Plugins an denselben Speicherort wie `clcl.ini`.

[**Tool in Einstellungen → Tool konfigurieren**](#tool-configuration)

### tool_text
Textbearbeitungswerkzeuge:
- To Lower - In kleinbuchstaben umwandeln
- To Upper - In GROSSBUCHSTABEN UMWANDELN
- Quotation - Als Zitat markieren, z.B. mit einem vorangestellten '>' und/oder einer Einrückung
- Un Quotation - Zitat von ausgewähltem Text entfernen
- Word Wrap - Text bei angegebener Spaltenbreite umbrechen
- &lt;TAG&gt;&lt;/TAG&gt; - Text mit benutzerdefinierten Tags umschließen
- Delete CRLF - Zeilenumbrüche entfernen
- Connection of text - Mehrere Verlaufselemente zu einem Text verbinden
- Edit - Text in einem Bearbeitungsfenster öffnen

### tool_utl
Dienstprogramme:
- Clear History - Alle Verlaufselemente löschen
- Clear Clipboard - Die Zwischenablage leeren
- Play Sound - Einen Ton abspielen, wenn Elemente zum Verlauf hinzugefügt werden
- Always on Top - Viewer-Fenster immer im Vordergrund halten
- Un Top - Einstellung "Immer im Vordergrund" entfernen
- Save of more items - Mehrere ausgewählte Elemente in Dateien speichern

### tool_clip
`tool_clip.dll` ist ein zusätzliches Plugin von https://github.com/wilfz/CLCL-tool_clip. Derzeit enthält es die folgenden Funktionen für Zwischenablageelemente:
- Ersetzen von Tabstopps und/oder Leerzeichen durch eine Zeichenfolge nach Wahl
- Ersetzen mit regulären Ausdrücken
- Elemente und Vorlagenordner in eine JSON-Datei exportieren
- Textelemente und -ordner aus einer JSON-Datei importieren und in Vorlagenordner zusammenführen
- Exportieren nach und Importieren von der Android-App [Clipper](https://play.google.com/store/apps/details?id=org.rojekti.clipper) bzw. [Clipper+](https://play.google.com/store/apps/details?id=fi.rojekti.clipper)
- Umwandlung von tabulatorgetrennten Daten in ein HTML-Snippet, zum anschließenden Einfügen in eine E-Mail, OneNote usw.
- Makros, Vorlagen mit erweiterten (Umgebungs-)Variablen, einfügen
- Aktuelles ausgewähltes Element an die Zwischenablage senden
- Aktuell ausgewähltes Element im Viewer anzeigen
- CLCL-Vorlagen in eine ODBC-Datenbank speichern und von dort laden 
- Fortsetzung folgt ...

### tool_python
[`tool_python`](https://github.com/wilfz/CLCL-tool_python#python-plugin-for-clcl-clipboard-manager) ermöglicht es Benutzern,  Python-Module mit eigenen Funktionen in CLCL einzubinden.  

Das Plugin enthält ein Beispielmodul _example.py_:
```python
#Python plugin functions must take string as 
#input parameter and return a string:

def main(input_text: str = ""):
    ret = "Hello Python!"
    return ret # return to CLCL

def ToLower(s: str = ""):
    return s.lower()

def ToUpper(s: str = ""):
    return s.upper()
```
Diese Funktiuonen sind relativ simpel, sollten aber ausreichen um einen Eindruck zu vermitteln.  

Da es eine Laufzeitumgebung für Python enthält, ist es relativ groß und unterliegt neben der MIT-Lizenz auch der Python-Lizenz. Deshalb ist es auch nicht im Setup- Programm enthalten, sondern liegt als separater Download mit einem eigenen [Setup](https://www.linguversa.de/clcl/download/setup_tool_python.exe) vor, um das Plugin in den gleichen Ordner wie CLCL zu installieren.

## Einstellungen

Öffnen Sie das Optionsfenster, indem Sie mit der rechten Maustaste auf den CLCL-Clip in der Taskleiste klicken oder es über das Hauptmenü des Viewers mit Ansicht -> Einstellungen aufrufen.  
In den verschiedenen Registerkarten des Optionsfensters können Sie CLCL gemäß Ihren Vorlieben und Bedürfnissen anpassen.  
Plus-Version: Durch Betätigen der Taste `F1` öffnet sich die Hilfe-Anzeige mit einer kurzen Erklärung zu der gerade aktiven Registerkarte.


### Verlaufseinstellungen

Auf der Registerkarte Verlauf der Einstellungen können Sie festlegen, wie viele Elemente behalten werden sollen, wann der Verlauf gespeichert werden soll, wie mit Duplikaten verfahren werden soll usw.

Wenn Sie die Anzahl der zu behaltenden Verlaufselemente auf einen Wert höher als die Standardanzahl von 30 erhöhen, empfiehlt es sich, auf der Registerkarte [Aktion](#aktion) die Anzahl der anzuzeigenden Menü-Elemenete zu begrenzen und/oder die Menüs mit Untermenüs zu organisieren. Siehe [hier](#mehr-verlaufselemente-und-wie-man-das-popup-menü-organisiert).

### Menüeinstellungen

Diese Registerkarte steuert, **wie** Menüelemente angezeigt werden.  
CLCL enthält mehrere verschiedene Menüs.

Verlauf- und Vorlagenelemente werden entsprechend der Option "Anzeigeformat des Menüs" angezeigt.  
Die angezeigten Zahlen beginnen bei 0, aber wenn Sie den Startwert ändern möchten, setzen Sie die Startnummer zwischen das % und das Zeichen.

Beispiele:

	%0d -> 0,1,2,3...
	%8x -> 8,9,a,b...
	%1n -> 1,2,3...8,9,0,1,2...
	%10B -> K,L,M,N...

Um ein Menü **hinzuzufügen** oder den **Inhalt** eines bestimmten Menüs zu konfigurieren, gehen Sie zur Option ["Aktion"](#aktion).

### Anzeige-Einstellungen

Der Viewer ist das Hauptfenster von CLCL. Auf der Registerkarte Anzeige können Sie festlegen, was in diesem Fenster angezeigt werden soll (Zwischenablage, Verlauf, Vorlagen) und ob die darüber liegenden Baumknoten kollabiert oder erweitert angezeigt werden sollen.

Mit dem Sprach-Auswahlfeld wählen Sie die Sprache des Viewers und des Optionsfensters.

(Wenn Sie versehentlich eine Ihnen unbekannte Sprache ausgewählt haben und das Sprach-Auswahlfeld nicht finden können, um es zurück in Ihre Sprache zu ändern, können Sie diese Einstellung auch rückgängig machen, indem Sie im Abschnitt `[main]` von `clcl.ini` den Eintrag `language` löschen.)

### Aktion
Aktionen und Menüs, die mit einem Hotkey oder einem Klick auf das Taskleistensymbol verknüpft sind, können auf der Registerkarte "Aktion" der Einstellungen konfiguriert werden.

"Aufrufart" bestimmt, wodurch eine Aktion aufgerufen wird.

Wenn Sie "Hotkey" angeben, legen Sie die Taste zum Aufrufen fest.
"Strg + Strg", "Shift + Shift" und "Alt + Alt" rufen die angegebene Aktion auf, wenn Sie die Taste zweimal drücken.

Wenn Sie "Menü" als Aktion angeben, können Sie mit der Checkbox "Einfügen" festlegen, dass das jeweils aus dem Menü ausgewählte Element nicht nur in die Zwischenablage gesendet, sondern auch die Einfügeaktion automatisch an die Anwendung gesendet wird, in der Sie gerade arbeiten.  

Wenn Sie beim Auswählen eines Menüelements die Umschalttaste gedrückt halten, wird es nicht eingefügt, sondern nur in die Zwischenablage gesendet.

Wenn Sie für die Aktion "Menü" zusätzlich "An Cursorposition anzeigen" aktiviert haben, erscheint das Popup-Menü an der Cursorposition des Editors.
Wenn Sie dies nicht festlegen, wird das Menü an der Mausposition angezeigt.

Wenn Sie das Menü als Aktion und Verlauf als Element auswählen, können Sie den Anzeigerbereich festlegen. Der Anzeigerbereich wird von 1 bis zum letzten im Menü anzuzeigenden Elements angegeben. Wenn Sie 0 als Startnummer angeben, ist dies dasselbe wie die Angabe von 1, und wenn Sie 0 als Endnummer angeben, ist dies dasselbe wie die Angabe der Anzahl der im Verlauf gespeicherten Elemente.

Wenn die Endnummer kleiner als die Startnummer ist, wird nichts angezeigt. Wenn die Endnummer größer als die Anzahl der im Verlauf gespeicherten Elemente ist, wird nur bis zum letzen gespeicherten Element angezeigt.

### Format
CLCL kann alle Zwischenablageformate verarbeiten, aber nicht registrierte Zwischenablageformate werden im Viewer als Binär-Dumps angezeigt.

Die Zwischenablageformate werden in den Einstellungen "Format" registriert. Das weiter oben registrierte Format hat Vorrang, und das Zwischenablageformat mit der höchsten Priorität unter den Elementen wird im Menü und Viewer angezeigt.

Um zu registrieren, klicken Sie auf "Hinzufügen", legen Sie den Formatnamen, die zu verarbeitende Format-Plugin-DLL und den Funktions-Header fest. Wenn Sie die DLL weglassen und die Schaltfläche zur Auswahl des Funktions-Headers drücken, wird eine Liste der in CLCL selbst integrierten Funktions-Header angezeigt.

Wenn Sie beispielsweise das CSV-Zwischenablageformat beim Kopieren in Excel als Text verarbeiten möchten, stellen Sie es wie folgt ein:
```
Formatname: CSV
DLL:
Funktions-Header: text_
```
Das Menü und der Viewer verarbeiten es dann als Text.

Format-Plugins sind DLLs, die es CLCL ermöglichen, spezielle Inhaltsformate benutzerfreundlich anzuzeigen.

CLCL wird mit dem Format-Plugin *fmt_rtf.dll* für das Rich-Text-Format geliefert.  
Dies kann durch Klicken auf "Hinzufügen", Auswählen der DLL und der rtf_ (Rich-Text-Format) Funktionsüberschrift aktiviert werden.

### Filter
Wählen Sie das Zwischenablageformat aus, indem Sie es in der Filteroption festlegen.

Wenn Sie "Alle Formate zum Verlauf hinzufügen" auswählen, werden alle Zwischenablageformate im Verlauf gespeichert, außer solchen, die explizit ausgeschlossen werden.

Wenn Sie umgekehrt "Alle Formate vom Verlauf ausschließen" auswählen, werden nur die Zwischenablageformate im Verlauf gespeichert, die explizit ausgwählt werden.  

Für die in der Filteroption festgelegten Zwischenablageformate können Sie zusätzlich die Größenbeschränkung beim Hinzufügen zum Verlauf festlegen. Daten, die die Grenzgröße überschreiten, werden nicht zum Verlauf hinzugefügt.

Wenn Sie das Zwischenablageformat in der Filteroption auf "Nicht speichern" setzen, wird es beim Schließen von CLCL nicht auf die Festplatte gespeichert.  
Sie können es beispielsweise so einstellen, dass Text und Bitmaps zum Verlauf hinzugefügt werden, aber beim Beenden von CLCL nur der Text dauerhaft gespeichert wird.  

### Fenstereinstellungen
Wenn Sie das Verhalten von CLCL je nach verwendeter Anwendung ändern möchten, legen Sie das Fenster und das Verhalten in der Option "Fenster" fest.

Geben Sie den Fenstertitel und den Klassennamen an und verwenden Sie "*" als beliebiges Zeichen.  
Geben Sie beispielsweise für Notepad Folgendes ein:

	Title: * - Notepad
	Class name: Notepad

Das Verhalten von CLCL ändert sich, wenn Notepad aktiviert ist.  
Entweder der Titel oder der Klassenname muss eingegeben werden, und wenn er nicht eingegeben wird, ist es dasselbe wie nur "*".

- Nicht zum Verlauf hinzufügen:
	Durch das Kopieren im festgelegten Fenster wird nichts zum Verlauf hinzugefügt.
	Wenn Sie diese Option für eine Anwendung festlegen, die Probleme verursacht, wenn sie zum Verlauf hinzugefügt wird, wird das Kopieren in dieser Anwendung ignoriert.

- Keinen Fokus setzen: 
	Der Fokus wird nach der Aktivierung des festgelegten Fensters nicht gesetzt.
	Wenn der Fokus beim Einfügen des ausgewählten Menüelements irgendwohin wandert und das Einfügen nicht korrekt erfolgt, kann die Angabe dieser Option dazu führen, dass es korrekt funktioniert.

- Auch bei abgebrochenem Werkzeug einfügen: 
	Bei Abbruch eines abbrechbaren Werkzeugs erfolgt normalerweise kein nachfolgendes Einfügen, aber wenn diese Option angegeben ist, wird das Einfügen auch bei Abbruch durchgeführt.
	Wenn Sie die Kopiertaste in den Tasteneinstellungen für jedes Fenster als Ausschneidetaste festgelegt haben, verhindert die Angabe dieser Option, dass die Zeichen verschwinden, selbst wenn Sie das Werkzeug abbrechen.

### Tasteneinstellungen für jedes Fenster
Wählen Sie das Verlauf- oder Vorlagenobjekt aus dem Hotkey aus, und die Einfügeaktion sendet automatisch die Einfügetaste an das Fenster.  
Standardmäßig wird Strg + V an alle Fenster gesendet, aber je nach Fenster kann die Einfüge-Taste eine andere Taste sein.

Wenn das Werkzeug mit einem Hotkey aufgerufen wird, wird die Kopiertaste (Strg + C) an das Fenster gesendet, um die Aktion Kopieren -> Werkzeugverarbeitung -> Einfügen auszuführen.

Die Kopier- und Einfügetasten für jedes Fenster werden in der Option "Tasten" festgelegt.
Legen Sie den Titel und den Klassennamen des einzustellenden Fensters fest und legen Sie die Kopier- und Einfügetasten fest.

Wenn die Kopier- und Einfügetasten nicht festgelegt sind, werden die Standardtasteneinstellungen verwendet.

Für ein Fenster können mehrere Tasten festgelegt werden. Wenn mehrere Tasten festgelegt sind, werden die Tasten der Reihenfolge nach von oben gesendet.

### Tool-Konfiguration

Wenn Sie eine Tool-DLL und einen Funktionsnamen auswählen, werden der Werkzeugname und der Aufruftyp automatisch festgelegt.

- Der Aufruftyp **Ansicht** ermöglicht die Ausführung über das Tool-Menü des Viewers.
- Der Aufruftyp **Aktionsmenü** ermöglicht die Ausführung über das Popup-Menü "Tools".
  - Die Sub-Option **Kopier- und Einfüge-Aktion setzen** kopiert die markierten Daten aus dem aktiven Fenster, führt das Tool auf den kopierten Daten aus und fügt die bearbeiteten Daten anschließend wieder in das aktive Fenster ein.  
Ohne diese Option wird das Tool auf dem neuesten Verlaufseintrag ausgeführt und in die Zwischenablage kopiert.  
Ein Rechtsklick auf ein Menüelement zeigt ein Popup-Menü mit den installierten Tools an, und das ausgewählte Tool wird auf diesem Element ausgeführt und sendet das Ergebnis in die Zwischenablage.

Ziehen Sie eine Plugin-DLL per Drag & Drop in das Tool-Listenfenster, um eine Liste von Tools anzuzeigen, die registriert werden können, und Sie können mehrere Tools auswählen, um sie alle auf einmal zu registrieren.  

## Befehlszeile
Beim Start von CLCL können Sie eine Befehlszeile angeben, um die Aktion nach dem Start festzulegen.

Wenn CLCL bereits ausgeführt wird, wird der Befehl an das bereits laufende CLCL gesendet.  
  
**Format:**

	CLCL.exe [/vownx]
		/v Viewer anzeigen
		/w Zwischenablage überwachen
		/n Überwachung der Zwischenablage deaktivieren
		/x Beenden

## Nützliche Hinweise

### QuickSearch

Obwohl CLCL Hunderte von Einträgen speichern kann, ist es schwierig, den Überblick zu behalten – selbst bei Verwendung kaskadierender Pop-up-Menüs.
Dasselbe gilt für Vorlagensammlungen, die Dutzende von Ordnern und Unterordnern umfassen.

Hier kommt die neue „QuickSearch“-Funktion ins Spiel.
Sie bietet das Look-and-Feel eines klassischen Popup-Menüs, ergänzt um ein Eingabefeld. Nach entsprechender Konfiguration in CLCL lässt sie sich aus jeder Anwendung heraus aufrufen, entweder per Tastenkombination (Hotkey) oder über das Tray-Icon.

Beim Eintippen eines Teilstrings des Inhalts oder des Titels des gesuchten Elements in das Eingabefeld, engt sich die Liste der angezeigten Elemente automatisch ein.
Sobald das gesuchte Element in der Liste sichtbar wird, genügt ein Klick, um es in die aktuell geöffnete Anwendung einzufügen.
Sehen Sie sich dazu die Bildschirmaufnahme an:
![Verwendung von QuickSearch](img/quicksearch_demo.gif)

Die Auswahl lässt sich natürlich auch bequem über die Pfeiltasten (nach oben/unten) und die Eingabetaste (Return) Ihrer Tastatur treffen.

#### Konfiguration
QuickSearch lässt sich leicht in das System-Tray-Menü integrieren und/oder mit einer Tastenkombination Ihrer Wahl verknüpfen. Klicken Sie auf die Bildschirmaufnahme, um mehr zu erfahren:
![Konfiguration von QuickSearch](img/clcl_quicksearch_configuration.gif)

### Größerer Verlauf und wie man das Popup-Menü organisiert
Im Register *Verlauf* der Einstellungen können Sie die maximale Anzahl der Elemente ändern, die im Verlauf vorgehalten werden sollen, z.B. von 30 auf 100. Aber ohne weitere Konfiguration wird das Popup-Menü recht überladen aussehen und ggf. einen Großteil des Bildschirms in Anspruch nehmen.

Es empfiehlt sich deshalb, den Verlauf in Untermenüs zu organisieren:  

![CLCL tray Menü](img/history_organized.png)

Um dies zu tun, wechseln Sie zur Registerkarte *Aktion*, wählen Sie das Menü *Linksklick auf das Tray* oder das Hotkey-Menü *Alt-C* aus und klicken Sie auf *Bearbeiten*.  
Ein neues Fenster öffnet sich, und dort wählen Sie *Verlauf/Aufsteigend*. Die meisten Steuerungen sind grau hinterlegt, aber Sie können den *Anzeigebereich* beispielsweise auf 0 bis 19 eingrenzen.

![Untermenü für den Verlauf erstellen](img/create_history_submenu.gif)  

Nun haben Sie 100 Elemente im Verlauf, aber das jeweilige Menü zeigt nur die ersten 19 an.  
Im linken Teil der Registerkarte *Aktion bearbeiten* scrollen Sie zum Ende und klicken auf *(Neuer Inhalt)*.  
Wählen Sie im Kombinationsfeld *Inhalt* "Popup-Menü" aus und fügen Sie "Verlauf 20 - 29" als *Titel* hinzu. Verschieben Sie jetzt das neue Popup-Menü (mit dem kleinen Dreieck unterhalb der linken Fensterhälfte) nach oben, bis es direkt unter *Verlauf/Aufsteigend* steht.  
Klicken Sie erneut auf *(Neuer Eintrag)*. Wählen Sie dieses Mal im Kombinationsfeld "Verlauf/Aufsteigend" aus und legen Sie den Anzeigebereich auf 20 bis 29 fest. Verschieben Sie den neuen Eintrag *Verlauf/Aufsteigend* nach oben, bis er direkt unter Ihrem neu erstellten Popup-Menü steht und etwas nach rechts eingerückt ist.  
Fahren Sie fort, so viele Popup-Menüs hinzuzufügen, wie Sie möchten. Sie können die Popups sogar kaskadieren, wie in den Screenshots gezeigt.  

![Aktion bearbeiten](img/edit_of_action.png)

## Credits
- Das CLCL-Hauptprogramm und die Plugins tool_text, tool_utl und tool_test sind Copyright (C) by [Ohno Tomoaki](https://nakka.com/), der es als Open Source freigegeben und 2024 unter die MIT-Lizenz gestellt hat
- Installer erstellt von WilfZim mit [Inno Setup](https://jrsoftware.org/isinfo.php)
- [Tool_clip-Plugin](https://github.com/wilfz/CLCL-tool_clip) von WilfZim verwendet Niels Lohmanns JSON-Bibliothek ( https://github.com/nlohmann/json ) für den Import und Export von Daten
- Übersetzung ins Vereinfachte Chinesisch von HeliusHao


## Änderungsverlauf

- Ver 2.2.0 -> Ver 2.2.1
	- Behebn eines Absturzes beim Doppelklicken auf das Tray-Icon.

- Ver 2.1.7 -> Ver 2.2.0
	- Unterstützung des Dark Mode von Windows.
	- Verbesserte Darstellung auf High-DPI Monitoren.
	- Neue Option, um das Popup-Menü anzuzeigen, ohne dass das aktuelle Fenster den Fokus verliert. 
	- Verbessertes Abspeicher beim Beenden.
	- UNICODE Unterstützung im Binary View hinzugefügt.
	- Anpassung, so Daten aus Password-Managern nicht im Verlauf gespeichert werden. (kashima-eyetech)

- Ver 2.1.6 -> 2.1.7
	- Neue Aufruftypen "Bei Verbindung zur Sitzung" und "Bei Trennung von Sitzung" in Tool-Konfiguration
	- Hilfe-Anzeige zum Aktionsmenü hinzugefügt
	- "Eintrag suchen" im Kontextmenü des Viewers hinzugefügt
	- "Schnellsuche" als neue Menü-Option und Aktion

- Ver 2.1.5 -> 2.1.6
	- Integration der CHM-Hilfe (Issue [nakkag#26](https://github.com"Bei Verbindung zur Sitzung"/nakkag/CLCL/issues/26)) und Aufruf des MS Help Viewers
	- Integration von Ohno Tomoakis Rich Text Format-Plugin
	- Ersetzen mit regulären Ausdrücken (tool_clip-Plugin)
	- Makros, Vorlagen mit erweiterten Variablen einfügen (tool_clip-Plugin)
	- Bugfix: Der Parameter `portable` in der Datei clcl_app.ini wird nun korrekt ausgewertet:  
    Im portablen Modus werden die INI-Dateien der Plugins im selben Ordner wie die Plugin-DLL gespeichert.

- Ver 2.1.4 -> 2.1.5
	- Einstellung hinzugefügt zur Verzögerung des Zugriffs auf die Zwischenablage (Merge aus Fork von Koichi-Kobayashi)
	- OS-Versionüberprüfung mit empfohlener Methode (Merge aus Fork von Koichi-Kobayashi)
	- Anzeige des Elementtitels nicht druckbarer Zeichen als Abkürzung oder ASCII-Code (Issue [nakkag#23](https://github.com/nakkag/CLCL/issues/23))
	- Kontextmenü zur Anzeige in Binärdarstellung hinzugefügt (nur für aktuelles Element)
	- Deutsche Benutzeroberfläche hinzugefügt
	- Ukrainische Benutzeroberfläche hinzugefügt
	- Chinesische Benutzeroberfläche hinzugefügt ([Issue #2](https://github.com/wilfz/CLCL/issues/2))
	- Sprache der Benutzeroberflächen im Register "Anzeige" der Einstellungen umschalten und in CLCL.ini speichern (funktioniert ab Windows Version Vista oder neuer)

- Ver 2.1.3 -> Ver 2.1.4
	- MIT-Lizenz
	- Plugins auf aktuelles OS aktualisiert
	- Japanische und Englische Plugin-Versionen zusammengeführt
	- Installer mit optionalen Plugins
	- Englische Dokumentation in README.md
	- HTML-Hilfe im Viewer-Menü
	- Umgebungsvariablen in Tool-Pfaden erweitern

- Ver 2.1.2 -> Ver 2.1.3
	- Letzte Version veröffentlicht unter https://nakka.com/soft/clcl/index_eng.html
	- Ändern des Taskleistensymbols beim Nichtüberwachen der Zwischenablage.
	- Verbesserung der Auf- und Abwärtsbewegungstasten in den Einstellungen.
	- Verbesserung, damit das Hauptfenster nicht angezeigt wird, wenn das Menü angezeigt wird.

- Ver 2.1.1 -> Ver 2.1.2
	- Hinzugefügt die Fähigkeit, durch Ziehen der Maus in Bildformaten (BITMAP, DIB) zu scrollen.

- Ver 2.1.0 -> Ver 2.1.1
	- Funktion zum Kopieren von Dateinamen und -pfaden in Dateiformaten hinzugefügt.
	- Einstellung hinzugefügt, um Einstellungen und Daten im selben Verzeichnis wie CLCL.exe zu platzieren.

- Ver 2.0.3 -> Ver 2.1.0
	- Hohe DPI-Unterstützung.
	- Unterstützung für PNG und JPEG beim Speichern von Bildformaten (BITMAP, DIB).
	- Bildformate (BITMAP, DIB) vergrößern oder verkleinern.
	  - Halten Sie die Strg-Taste gedrückt und verwenden Sie das Mausrad oder Strg+↑↓, um die Vergrößerung zu ändern.
	- Binärdateien werden nicht mehr in japanischen und englischen Versionen getrennt.
	- Ändern des Speicherorts für Einstellungen, um in einem separaten Bereich für jeden Benutzer gespeichert zu werden.
	  -	Unter Windows 10/11, "C:\Users\(Benutzername)\AppData\Local\CLCL"
		(Wird beim ersten Start automatisch migriert)

- Ver 2.0.2 -> Ver 2.0.3
	- Rechtschreibfehler in den Einstellungsfenstern der englischen Version korrigiert.

- Ver 2.0.1 -> Ver 2.0.2
	- Unicode-Unterstützung in Einstellungsdateien.
	  - Wenn Sie einmal mit Ver 2.0.2 starten, wird die Einstellungsdatei Unicode.
		Die Dateien vor der Konvertierung werden mit den Dateinamen "general.ini.back" und "clcl.ini.back" gesichert.
	Wenn Sie zu einer älteren Version zurückkehren möchten, verwenden Sie die obigen Dateien.
	- Die Startfunktion wurde von WinMain auf wWinMain geändert.

- Ver 2.0.0 -> Ver 2.0.1
	- Incorporierte Korrekturcode von K. Takata
	  - Unterstützt Unicode-Ersatzpaare
	  - Wiederholte Verarbeitung, wenn das Hinzufügen zum Tasktray fehlschlägt
	  - Verhindert aufeinanderfolgende doppelte Starts und doppelte Starts von einem anderen Benutzer
	- Behoben, dass die Farbe von Elementen, die in den "Aktion"-Einstellungen deaktiviert sind, manchmal unsichtbar wurde
	- Verbesserung des Taskleistensymbols, wenn das Überwachen der Zwischenablage gestoppt ist.

- Ver 1.1.2 -> Ver 2.0.0
	- Unterstützt Unicode. Dadurch werden ältere Versionen von Windows nicht mehr unterstützt.
	  - Die Einstellungen der älteren Version werden übernommen, aber Tools und Plugin-Formate müssen Unicode-kompatibel sein.
	- Verhalten geändert, um die Datei zu kopieren, wenn die Speicherung erfolgreich ist, damit Vorlagenobjekte nicht gelöscht werden, wenn der Speichervorgang fehlschlägt, wenn CLCL geschlossen wird.
	- Design des Symbols in der Taskleiste geändert.
	- Standardverhalten bei einem Klick auf die Taskleiste geändert.
	  - Das Menü wird jetzt angezeigt, wenn auf das Taskleistensymbol geklickt wird, um Tablet-Betrieb zu berücksichtigen.

- Ver 1.1.1 -> Ver 1.1.2
	- Behoben, dass der Hintergrund des Tooltips in einer 256-Farben-Umgebung nicht korrekt angezeigt wurde.
	- Optimierung der Menüdarstellung.
	- Optimierung der Textdarstellung im Viewer.

- Ver 1.1.0 -> Ver 1.1.1
	- Verbesserung des Verhaltens beim Auswählen der Zeichenposition mit der Maus in der TEXT-Bearbeitung.
	- Behoben, dass beim Schließen kein Verlauf gespeichert wurde, wenn der Filter aktiv war.
	- Möglichkeit, Elemente zwischen Verlauf und Vorlagen zu verschieben.
	- Änderung der Methode zur Anzeige von Miniaturbildern.
	- Änderung des Standardwerts der Wartezeit für Kopieren und Einfügen auf 100.
	- Behoben, dass der Speicher nicht freigegeben wurde.

- Ver 1.0.9 -> Ver 1.1.0
	- Behoben, dass das Menü in einer Schleife war, wenn Strg+V oder andere Tasten als Hotkey zum Öffnen des Menüs festgelegt waren.
	- Jetzt ist es möglich, die Windows-Taste oder die Leertaste als Hotkey für Aktionen und Tools festzulegen.

- Ver 1.0.8 -> Ver 1.0.9
	- Behoben, dass die Anzeige verzerrt wurde, wenn die Bestätigungsnachricht für dasselbe Format beim Erstellen eines neuen Elements im Viewer abgebrochen wurde.
	- Behoben, dass das bearbeitete Format unabhängig von der Priorität immer angezeigt wurde, wenn der Inhalt des Formats im Viewer bearbeitet wurde.
	- Behoben, dass die Statusleiste nicht aktualisiert wurde, wenn ein Element im Viewer bearbeitet wurde.
	- Behoben, dass sich die Position der japanischen Eingabe beim Bearbeiten von TEXT verschob.

- Ver 1.0.7 -> Ver 1.0.8
	- Ändern Sie den INI-Dateinamen (user.ini -> clcl.ini)
	 Der Dateiname wird beim Start automatisch geändert.
	- Anzeige und Bearbeitung des TEXT-Formats von Rich-Edit-Steuerelement auf unser eigenes.
	- Verbesserung des Verhaltens des Viewers beim Aufrufen von Tools aus dem Viewer.
	- Erweiterte Funktion der binären Anzeige.
	- Sonstiges

- Ver 1.0.6 -> Ver 1.0.7
	- Beim Auswählen eines Elements im Menü bei gedrückter Umschalttaste wird es nur in die Zwischenablage gesendet, ohne eingefügt zu werden.
	- Hinzugefügt eine Befehlszeilenoption.
	- Behoben, dass Tasten nicht an einige Fenster gesendet werden konnten.
	- Erhöhte die Standardeinstellungen für das Verhalten der Liste in den Anzeige-Einstellungen.

- Ver 1.0.5 -> Ver 1.0.6
	- Möglichkeit, Elemente in den Verhaltenseinstellungen zu kopieren.
	- Änderung der Anzahl der angezeigten Verlaufselemente auf den Anzeigeumfang in den Verhaltenseinstellungen.
	- Kleinere Anpassungen an den Einstellungs-Fenstern.

- Ver 1.0.4 -> Ver 1.0.5
	- Möglichkeit, Vorlagenobjekten einen Hotkey zuzuweisen.
	- Hotkeys im Menü sichtbar gemacht.
	- Die Position des Viewers wird beim Starten nicht mehr angepasst.
	- Die Position und Größe werden jetzt beim Schließen des Viewers zwangsläufig gespeichert.
	- 0x7F wird nicht mehr im Zeichenanzeigebereich angezeigt, wenn Binärdaten angezeigt werden.
	- Dateien können jetzt geöffnet werden, auch wenn sie gesperrt sind.

- Ver 1.0.3 -> Ver 1.0.4
	- Dateien können jetzt automatisch als externe Anwendungen registriert werden, wenn sie in den Drag & Drop-Bereich des Einstellungsscreens gezogen und dort abgelegt werden.

- Ver 1.0.2 -> Ver 1.0.3
	- Behoben, dass der Verlauf unter Windows XP nicht korrekt gespeichert wurde, wenn das Betriebssystem heruntergefahren wurde.

- Ver 1.0.1 -> Ver 1.0.2
	- Verbesserung des Zeitpunkts der Freigabe von mit Menüs verbundenem Speicher.
	- Ordnersymbole verwenden jetzt Systemsymbole.

- Ver 1.0.0 -> Ver 1.0.1
	- Einstellungen für jedes Fenster hinzugefügt.
	- Behoben, dass der Text je nach Umgebung nach Änderung der Schriftart nicht neu gezeichnet wurde.
	- Behoben, dass das Einfügen in einigen Programmen nicht möglich war.
	- Behoben, dass alle Fenster angesprochen wurden, wenn nur einer der Fenstertitel und der Klassenname in den Fenster- und Tasteneinstellungen angegeben wurde.

- Ver 0.2.0 -> Ver 1.0.0
	- Wiederherstellung
	 Daten und Einstellungen werden nicht übernommen.
	 Wenn Sie Vorlagenobjekte von Ver 0.2.0 nach Ver 1.0.0 migrieren möchten, starten Sie beide und senden Sie die Vorlagenobjekte von Ver 0.2.0 in die Zwischenablage und fügen Sie sie zur Verlaufsliste von Ver 1.0.0 hinzu. Registrieren Sie anschließend die Elemente aus dem Verlauf, der zu Ver 1.0.0 hinzugefügt wurde, als Vorlagenobjekte von Ver 1.0.0. Tools, die mit Ver 0.2.0 kompatibel sind, können auch mit Ver 1.0.0 verwendet werden. (Einige Einschränkungen gelten) 


Der Autor ist nicht verantwortlich für durch dieses Programm verursachte Probleme. Es wird dringend empfohlen, wichtige Dateien zu sichern. 

Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved. https://nakka.com/soft/clcl/index_eng.html  
Quellcode: https://github.com/nakkag/CLCL  
2025 - 2026 MIT License.  

DoKumentation und Downloads: https://linguversa.de/clcl  
Erweiterter Quellcode: https://github.com/wilfz/CLCL  
