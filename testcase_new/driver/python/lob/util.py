from random import Random
import string

def random_str(length=1024):
   s=""
   r=Random()
   for i in range(length):
      s+=r.choice(string.ascii_letters+string.digits)

   return s

def get_md5(text):
   import hashlib
   m2=hashlib.md5()
   m2.update(text)
   return m2.hexdigest()

