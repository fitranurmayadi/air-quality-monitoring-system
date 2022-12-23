#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][5] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char monthName[12][5] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

String getTimeRTC() {
  DateTime now = rtc.now();
  String jam, menit;
  if (now.hour() < 10) {
    jam = "0" + String(now.hour());
  } else {
    jam = String(now.hour());
  }

  if (now.minute() < 10) {
    menit = "0" + String(now.minute());
  } else {
    menit = String(now.minute());
  }
  String timeNow = jam + ":" + menit;
  return timeNow;
}

String getDateRTC() {
  DateTime now = rtc.now();

  String dateNow = String(daysOfTheWeek[now.dayOfTheWeek()]) + ", " + String(now.day()) + " " + String(monthName[now.month()]) + " " + String(now.year());
  return dateNow;
}
