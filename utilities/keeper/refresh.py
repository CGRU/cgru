import cgruconfig
import nimby

# Refresh interval in seconds:
Interval = 36

def refresh():
   cgruconfig.Config()
   nimby.refresh()
