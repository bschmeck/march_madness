#! /usr/bin/python

from HTMLParser import HTMLParser, HTMLParseError
from mechanize import Browser

import getopt
import re
import sys

LOGIN_URL = "https://login.yahoo.com/config/login"
STANDINGS_URL = "http://tournament.fantasysports.yahoo.com/t1/group/%s/standings"

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg
        
class MyParser(HTMLParser):
    def attrs_to_dict(self, attrs):
        ret = {}
        for key, value in attrs:
            ret[key] = value
        return ret
        
class YahooScoreboardParser(MyParser):
    HREF_STARTSWITH = "/t1/"
    TABLE_CLASS = "game-table"
    BRACKET_PREFIX = "http://tournament.fantasysports.yahoo.com"
    def __init__(self):
        MyParser.__init__(self)
        self.teams = dict()
        self.cur_url = None
        self.capture = False
        
    def handle_starttag(self, tag, attrs):
        # print tag
        if tag == 'table':
            for key, value in attrs:
                if key != "class": continue
                self.capture = value == self.TABLE_CLASS
        elif self.capture and tag == 'a':
            for key, value in attrs:
                if key != "href": continue
                if value.startswith(self.HREF_STARTSWITH):
                    self.cur_url = value
                    
    def handle_endtag(self, tag):
        if tag == "table": self.capture = False

    def handle_data(self, data):
        if self.capture and self.cur_url:
            self.teams[data] = self.BRACKET_PREFIX + self.cur_url
            self.cur_url = None

class YahooBracketParser(MyParser):
    def __init__(self):
        MyParser.__init__(self)
        self.regions = {}
        self.cur_region = None
        self.cur_region_name = None
        self.div_count = 0
        self.cur_pick = None
        
    def handle_starttag(self, tag, attrs):
        if tag == "div":
            if self.cur_region_name: self.div_count += 1
            attrs = self.attrs_to_dict(attrs)
            m = re.match("region-(\d)", attrs.get("id", ""))
            if m:
                self.cur_region_name = ["FF", "UL", "UR", "LL", "LR"][int(m.group(1))]
                self.cur_region = []
                self.div_count = 1
        elif tag == "strong":
            if self.cur_region_name:
                attrs = self.attrs_to_dict(attrs)
                if "ysf-tpe-user-pick" in attrs.get("class", "").split(" "):
                    self.cur_pick = ""
        
    def handle_endtag(self, tag):
        if tag == "div" and self.cur_region_name:
            self.div_count -= 1
            if self.div_count == 0:
                self.regions[self.cur_region_name] = self.cur_region
                self.cur_region_name = None
        elif tag == "strong" and self.cur_pick is not None:
            self.cur_region.append(self.cur_pick)
            self.cur_pick = None
            
    def handle_data(self, data):
        if self.cur_pick is not None:
            self.cur_pick += data + " "
            
class Scraper:
    def __init__(self, username, password, group_id):
        self.username = username
        self.password = password
        self.group_id = group_id
        self.browser = None
        
    def content(self, url):
        print "Get", url
        res = self.browser.open(url)
        content = res.read()
        # Yahoo occasionally returns a link tag with no space between the style
        # attribute string and the href attribute tag.  Fix that.
        content = content.replace(';"href', ';" href')
        return content
    
    def scrape(self):
        url = LOGIN_URL
        self.browser = Browser()
        print "Get", url
        res = self.browser.open(url)
        self.browser.select_form(name="login_form")
        self.browser.form["login"] = self.username
        self.browser.form["passwd"] = self.password
        print "Submit form"
        self.browser.submit()

        teams = {}
        while len(teams.keys()) == 0:
            url = STANDINGS_URL % self.group_id
            content = self.content(url)
            p = YahooScoreboardParser()
            p.feed(content)
            teams = p.teams

        for name, url in teams.iteritems():
            picks = {}
            while len(picks.keys()) == 0:
                content = self.content(url)

                p = YahooBracketParser()
                try:
                    p.feed(content)
                except HTMLParseError:
                    pass
                picks = p.regions
            for region, winners in picks.iteritems():
                print name, region, winners

if __name__ == "__main__":
    try:
        try:
            opts, args = getopt.getopt(sys.argv[1:], "g:p:u:")
        except getopt.error, msg:
            raise Usage(msg)

        group_id = None
        password = None
        username = None
        for option, arg in opts:
            if option == "-g":
                group_id = arg
            elif option == "-p":
                password = arg
            elif option == "-u":
                tmp = arg.split(":")
                username = tmp[0]
                if len(tmp) == 2: password = tmp[1]
        if not group_id and not username and not password:
            raise Usage("Must specify a group number, username and password")
    except Usage, err:
        print >>sys.stderr, err.msg
        sys.exit(2)
    scraper = Scraper(username, password, group_id)
    sys.exit(scraper.scrape())
