
SWETARGET = svenska/FrexxEd.catalog
GERTARGET = deutsch/FrexxEd.catalog
ITATARGET = italiano/FrexxEd.catalog
DUTTARGET = nederlands/FrexxEd.catalog
POLTARGET = polski/FrexxEd.catalog
DANTARGET = dansk/FrexxEd.catalog
FRATARGET = francais/FrexxEd.catalog

SWEDISH = FrexxEd_swedish.ct
ITALIAN = FrexxEd_italian.ct
GERMAN  = FrexxEd_german.ct
DUTCH   = FrexxEd_dutch.ct
POLISH  = FrexxEd_polish.ct
DANISH  = FrexxEd_danish.ct
FRENSH  = FrexxEd_french.ct

All: $(SWETARGET) $(GERTARGET) $(ITATARGET) $(DUTTARGET) $(POLTARGET) \
     $(DANTARGET) $(FRATARGET)

$(SWETARGET): FrexxEd.cd $(SWEDISH)
  CatComp FrexxEd.cd $(SWEDISH) CATALOG $(SWETARGET)

$(GERTARGET): FrexxEd.cd $(GERMAN)
  CatComp FrexxEd.cd $(GERMAN) CATALOG $(GERTARGET)

$(ITATARGET): FrexxEd.cd $(ITALIAN)
  CatComp FrexxEd.cd $(ITALIAN) CATALOG $(ITATARGET)

$(DUTTARGET): FrexxEd.cd $(DUTCH)
  CatComp FrexxEd.cd $(DUTCH) CATALOG $(DUTTARGET)

$(POLTARGET): FrexxEd.cd $(POLISH)
  CatComp FrexxEd.cd $(POLISH) CATALOG $(POLTARGET)

$(DANTARGET): FrexxEd.cd $(DANISH)
  CatComp FrexxEd.cd $(DANISH) CATALOG $(DANTARGET)

$(FRATARGET): FrexxEd.cd $(FRENSH)
  CatComp FrexxEd.cd $(FRENSH) CATALOG $(FRATARGET)
