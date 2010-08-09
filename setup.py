from setuptools import setup
from pv import __version__

setup(name="photovoltaic",
      version=__version__,
      description="A MongoDB/Solr search connector",
      author="Michael Stephens",
      author_email="me@mikej.st",
      license="AGPLv3",
      url="http://github.com/mikejs/photovoltaic",
      platforms=["any"],
      classifiers=["Development Status :: 4 - Beta",
                   "Intended Audience :: Developers",
                   "Natural Language :: English",
                   "Operating System :: OS Independent",
                   "Programming Language :: Python"])
