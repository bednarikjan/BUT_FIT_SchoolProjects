#! /usr/bin/perl

#CSV:xbedna45

# Projekt: IPP - CSV
# Autor: Jan Bednarik
# Login: xbedna45
# Datum 25.3.2012

use strict;
use warnings;
use locale;   # Pouziti jazykoveho nastaveni terminalu

use Text::CSV;
use XML::Writer;

# Chybove stavy skriptu
use constant {
  E_OK => 0,
  E_PARAMS => 1,
  E_READ => 2,
  E_WRITE => 3,
  E_CSV => 4,
  E_OPEN => 5,
  E_XML_TAG => 30,
  E_XML_TAG2 => 31,
  E_CLMNS_NUM => 32,
  E_SEPARATOR => 100,
  E_EOF => 101,
};

# Nazvy parametru
use constant {
  P_HELP => "help",
  P_INPUT => "input",
  P_OUTPUT => "output",
  P_N => "n",
  P_H => "h",
  P_I => "i",
  P_E => "e",
  P_R => "r",
  P_S => "s",
  P_L => "l",
  P_ALL_COLUMNS => "all-columns",
  P_START => "start",
  P_MISSING_VALUE => "missing-value",
};

# Hlavicka XML
use constant {XML_HEAD => "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"};

# Chybova hlaseni skriptu
my %errorMsg = (
  1   => "CHYBA: Chybne zadane parametry programu.\n",
  2   => "CHYBA: Nelze otevrit vstupni soubor.\n",
  3   => "CHYBA: Nelze otevrit vystupni soubor.\n",
  4   => "CHYBA: Nekorektni format vstupniho CSV souboru.\n",
  5   => "CHYBA: Nelze otevrit STDIN nebo STDOUT.\n",
  30  => "CHYBA: Nevalidni XML tag (parametry -r -l).\n",
  31  => "CHYBA: Nevalidni XML tag (parametr -h).\n",
  32  => "CHYBA: Chybny pocet sloupcu na neprvnim radku.\n",
  100 => "CHYBA: Nevaidni separator.\n",
);

##########################################################################################
### Deklarace funkci

sub Help();
sub Terminate($);
sub GetOpts(\%);
sub VerifyParams(\%\@);
sub GetCSVLine(\$);
sub ValidateTags(\@);

##########################################################################################
### Definice funkci

# Vypis napovedy a konec.
sub Help() {
  print <<END_OF_HELPMSG
Napoveda: CSV2XML
-----------------
Skript pro konverzi formatu CSV do XML.

Parametry:

--help                       napoveda
--input=filename             vstupni soubor(CSV)
--output=filename            vystupni soubor(XML)
-n                           negenerovat XML hlavicku
-r=root-element              nastaveni root elementu
-s=separator                 nastaveni separatoru
-h                           pouzit prvni radek CSV souboru jako hlavicku
-l=line-element              nastaveni radkoveho elementu
-i -line-elements            vlozeni indexovani radku
-start=n -i -l=line-elements nastaveni pocatecniho cisla
-e --error-recovery          zotaveni z chybneho poctu sloupcu na neprvnim radku 
-e --missing-value=val       nastaveni retezce dolneneho za chybejici sloupcec
-e --all-columns             neignorovat prebyvajici sloupce
END_OF_HELPMSG
}

# Vypis chyboveho hlaseni a ukonceni programu.
sub Terminate($) {
  print STDERR $errorMsg{$_[0]};
  exit $_[0];
}

# Zpracuje vstupni parametry programu.
# \% odkaz na hash - ulozeni hodnot parametru.
# return 0 - OK
# return 1 - Chybne zadane parametry
sub GetOpts(\%) {
  my $aux;
  my $params = shift(@_);
  
  foreach my $p (@ARGV) {
    # Kontrola, zda parametr zacina pomlckou '-'
    if($p !~ /^-/) {return E_PARAMS;}
    # Odstraneni -- nebo - z parametru
    substr($p, 0, ($p =~ /^--/) ? 2 : 1, "");
    
    # Vyber parametru.
    CASE:{
      # --help
      $p =~ /^help$/ and do {
	if(scalar @ARGV > 1) {return E_PARAMS;}
	$params->{+P_HELP} = 1;
	last CASE;};

      # -n, -h, -i, -e, --all-columns
      $p =~ /(^[nhie]$)|(^all-columns$)/ and do {
	if(exists($params->{$p})) {return E_PARAMS};
	$params->{$p} += 1;
	last CASE;};

      # --error-recovery
      $p =~ /^error-recovery$/ and do {
	if (exists($params->{+P_E})) {return E_PARAMS};
	$params->{+P_E} += 1;
	last CASE;};
      
      # --input=filename, --output=filename, -r, -s, -l, --start, --missing-value
      $p =~ /(^input=)|(^output=)|(^r=)|(^s=)|(^l=)|(^start=)|(^missing-value=)/ and do {
	$p =~ /^(.+)=/ and $aux = $1;
	$p =~ s/.+=//;
	if(exists($params->{$aux})) {return E_PARAMS};
	$params->{$aux} = $p;
	last CASE;};

      # neznamy parametr
      do {return E_PARAMS;}
    }
  }
  
  return E_OK;
}

# Zkontroluje spravnost kombinaci a hodnot parametru
# \% - odkaz na hash
# \@ - odkaz na pole pro elementy z hlvaicky CSV (parametr -h)
# return 0 - OK
# return 1 - Chybne kombinace nebo hodnoty
sub VerifyParams(\%\@) {
  my $par = shift(@_);
  my $head = shift(@_);

  # --start and -i and --start=val >= 0
  if(exists($par->{+P_START})) {
    if(!exists($par->{+P_I}) or ($par->{+P_START} !~ /^[+]?\d+$/) or ($par->{+P_START} < 0)) {
      return E_PARAMS;
    }
  }

  # -i and -l
  if(exists($par->{+P_I}) and !exists($par->{+P_L})) {return E_PARAMS;}

  # --missing-value and -e, spravna hodnota val
  if(exists($par->{+P_MISSING_VALUE})) {
    if(!exists($par->{+P_E})) {return E_PARAMS;}
  }
  
  # --all-columns and -e
  if(exists($par->{+P_ALL_COLUMNS}) and !exists($par->{+P_E})) {return E_PARAMS;}
  
  # -r -> validita XML znacky
  if(exists($par->{+P_R})) {
    # Mam tu ten povoleny interval napsany v negaci a obracene, protoze pri primem zapisu 
    # povoleneho intervalu to nefunguje.
    if($par->{+P_R} !~ /^[^\x00-\x39\x3b-\x40\x5b-\x5e\x60\x7b-\x7f][^\x00-\x2c\x2f\x3b-\x40\x5b-\x5e\x60\x7b-\x7f]*$/) {
      return E_XML_TAG;
    }
  }

  # -l -> validita XML znacky
  if(exists($par->{+P_L})) {
    if($par->{+P_L} !~ /^[^\x00-\x39\x3b-\x40\x5b-\x5e\x60\x7b-\x7f][^\x00-\x2c\x2f\x3b-\x40\x5b-\x5e\x60\x7b-\x7f]*$/) {
      return E_XML_TAG;
    }
  } else {
    # implicitni hodnota.
    $par->{+P_L} = "row";
  }

  # -s -> validita separatoru
  if(exists($par->{+P_S})) {
    if($par->{+P_S} =~ /^TAB$/) {$par->{+P_S} = "\t";}
    if($par->{+P_S} !~ /^[\x20-\x21\x23-\x7e,\r\n\t]$/) {Terminate(E_SEPARATOR);}
  } else {
    # Nastaveni implicitniho separatoru
    $par->{+P_S} =",";
  }
  return E_OK;
}

# ziskani jednoho radku z CSV
# \$ - odkaz na skalar pro ulozeni radku.
sub GetCSVLine(\$) {
  my $line = shift(@_);
  my $counter = 0;

  $/ = "\r\n";
  if(eof(INPUT)) {return E_EOF;}
  $$line = <INPUT>;
  #print $$line;

  for($$line =~ /"/g) {$counter++;}
  # v radku lichy pocet ", zrejme CRLF uvnitr uvozovek -> spojit s dalsim radkem.
  while($counter & 1) {
    last if(eof(INPUT));
    $counter = 0;
    $$line = $$line . <INPUT>;
    for($$line =~ /"/g) {$counter++;}
  }
  #print "lajna = $$line<-";
  if(($$line =~ /\r\n$/) and eof(INPUT)) {Terminate(E_CSV);}
  else{$$line =~ s/\r\n$//;}

  return E_OK;
}

# Validace jmen sloupcu -> XML tagu
# \@ - pole nazvu sloupcu
sub ValidateTags(\@) {
  my $tags = shift(@_);

  if(scalar @$tags == 0) {return E_XML_TAG2;}

  for(my $i = 0; $i < scalar @$tags; $i++) {
    @$tags[$i] =~ s/[\x00-\x2c\x2f\x3b-\x40\x5b-\x5e\x60\x7b-\x7f]/-/g;
    if(@$tags[$i] !~ /^[^\x00-\x39\x3b-\x40\x5b-\x5e\x60\x7b-\x7f][^\x00-\x2c\x2f\x3b-\x40\x5b-\x5e\x60\x7b-\x7f]*$/) {
      return E_XML_TAG2;
    }
  }

  return E_OK;
}


##########################################################################################
##########################################################################################
### Zacatek skriptu
#

my $err;
my @header;
my %params;
keys(%params) = 13;
my @clmnsNames;
my $line;

# Tvorba aliasu pro stdin
*INPUT = *STDIN;
*OUTPUT = *STDOUT;

# Zpracovani parametru prikazove radky
if(GetOpts(%params) != E_OK) {Terminate(E_PARAMS)};
if(exists($params{+P_HELP})) {Help(); exit 0;}
if(($err = VerifyParams(%params,@header)) != E_OK) {Terminate($err);}

# Oteverni souboru pro zapis a cteni.
if(exists($params{+P_INPUT}) and !open(INPUT, "<$params{+P_INPUT}")) {Terminate(E_READ);}
if(exists($params{+P_OUTPUT}) and !open(OUTPUT, ">$params{+P_OUTPUT}")) {Terminate(E_WRITE);}

# Objekt tridy Text::CSV
my $csv = Text::CSV->new ({
  binary => 1,
  eol => "\r\n",
  sep_char => $params{+P_S},
  verbatim => 1,
}) or die "Cannot use CSV: " . Text::CSV->error_diag ();

my $clmnCnt;	# Pocet sloupcu podle prvniho radku
my $resLine;    # Navratova hodnota fce GetCSVLine
my @columns;    # Pole pro prvni obsah poli(sloupcu)

# Nacteni prvniho radku.
if(($resLine = GetCSVLine($line)) == E_EOF) {$line = "";}
if(!$csv->parse($line)) {Terminate(E_CSV);}
@clmnsNames = $csv->fields();
$clmnCnt = scalar @clmnsNames;
# Pokud byl zadan parametr -h, musi se validovat tagy.
if(exists($params{+P_H})) {	
  if(ValidateTags(@clmnsNames) != E_OK) {Terminate(E_XML_TAG2);}
  # Nacteni dalsiho radku.
  $resLine = GetCSVLine($line);
  if($resLine != E_EOF) {
    if(!$csv->parse($line)) {Terminate(E_CSV);}
    @columns = $csv->fields();
  }
} else {
  @columns = @clmnsNames;
}

# Objekt tridy XML::Writer
my $writer = XML::Writer->new(
DATA_MODE => 1,
DATA_INDENT => 2,
UNSAFE => 1,
);

#XML lavicka
if(!exists($params{+P_N})) {print OUTPUT XML_HEAD;}
#root-element oteviraci znacka
if(exists($params{+P_R})) {$writer->startTag($params{+P_R});}

my $clmnNum;
my $idx = (exists($params{+P_START})) ? ($params{+P_START}) : (1);
my $i;

# ad-hoc reseni pro pripad, kdy je na vstupu prazdny soubor.. lame reseni, ale tenhle
# pripad jsem si uvedomil az moc pozde.
if($resLine == E_EOF) {
  if(exists($params{+P_I})) {$writer->startTag($params{+P_L}, 'index' => $idx);}
  else {$writer->startTag($params{+P_L});}

  $writer->startTag("col1");
  $writer->characters("");
  $writer->endTag("col1");

  $writer->endTag($params{+P_L});
}

while($resLine != E_EOF) {
  # oteviraci tag pro radek
  if(exists($params{+P_I})) {$writer->startTag($params{+P_L}, 'index' => $idx);}
  else {$writer->startTag($params{+P_L});}
  
  $i = 0;
  for(@columns) {
    if($i == $clmnCnt) {$i++; last;}
    $writer->startTag((exists($params{+P_H})) ? $clmnsNames[$i] : ("col" . ($i+1)));
    $writer->characters($_);
    $writer->endTag((exists($params{+P_H})) ? $clmnsNames[$i] : ("col" . ($i+1)));
    $i++;
  }

  # error-recovery
  if(exists($params{+P_E})) {
    # sloupce chybi
    if($i < $clmnCnt) {
      while($i < $clmnCnt) {
        #$writer->startTag("col" . ($i+1));
	$writer->startTag((exists($params{+P_H})) ? $clmnsNames[$i] : ("col" . ($i+1)));
	$writer->characters((exists($params{+P_MISSING_VALUE})) ? ($params{+P_MISSING_VALUE}) : (""));
	$writer->endTag((exists($params{+P_H})) ? $clmnsNames[$i] : ("col" . ($i+1)));
	$i++;
      }
    } elsif($i > $clmnCnt and exists($params{+P_ALL_COLUMNS})) {
      while($i <= scalar @columns) {
        $writer->startTag("col" . ($i));
	$writer->characters($columns[$i-1]);
	$writer->endTag("col" . ($i));
	$i++;
      }
    }
  } elsif($i != $clmnCnt) {Terminate(E_CLMNS_NUM);}
  
  # zaviraci tag pro radek
  $writer->endTag($params{+P_L});
  $idx++;

  # ziskani dalsiho radku CSV
  if(($resLine = GetCSVLine($line)) != E_EOF) {
    if(!$csv->parse($line)) {Terminate(E_CSV);}
    @columns = $csv->fields();
  }
}

#root-element zaviraci znacka
if(exists($params{+P_R})) {$writer->endTag($params{+P_R});}

# Skript skoncil v poradku.
exit 0;
