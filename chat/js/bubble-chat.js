var question_length=10;
var count = question_length;
var sub
var starting_width
var width 
var quote = 0
var quotes = ['"What\'s your favorite dance move?"',
  '"Do you think LeBron James is HOT?"',
 '"What\'s your favorite outfit?"', 
 '"Do you want to make a movie?"', 
 '"Why is the rum gone?"']

function timer()
{
  count=count-1;
  $(".timer").html(count);
  width=width-sub;
  $(".bar").width(width);
  if (count <= 0)
  {
    $(".timer").html(0);
    $(".bar").width(width);
    clearInterval(counter);
    width = starting_width;
    $(".timer").html(0);
    count = question_length;
    $("#current-question-text").html(quotes[quote]);
    quote++;
    if (quote>=quotes.length) quote = 0;
    counter = setInterval(timer, 1000); //ms
  }

}

function calcWidth(classname) 
{ 
  console.log("in calcwidth")
  width = $('#'+classname+'').width();
  console.log(width)
  return width
}

function increment() 
{
  var str = $('#yeah-btn').text();
  var int = parseInt(str.substr(6));
  int++;
  $('#yeah-btn').text("Yeah: "+int)
}

function incrementYeah() 
{
  var str = $('#yeah-btn').text();
  var int = parseInt(str.substr(6));
  int++;
  $('#yeah-btn').text("Yeah: "+int)
}

function incrementBoo() 
{
  var str = $('#boo-btn').text();
  var int = parseInt(str.substr(5));
  int++;
  $('#boo-btn').text("Boo: "+int)
}

function incQ1() {
  var str = $('#Q1votes').text();
  var votes = parseInt(str);
  $('#Q1votes').text(votes+1)
}
function decQ1() {
  var str = $('#Q1votes').text();
  var votes = parseInt(str);
    $('#Q1votes').text(votes-1)
}

function incQ2() {
  var str = $('#Q2votes').text();
  var votes = parseInt(str);
  $('#Q2votes').text(votes+1)
}
function decQ2() {
  var str = $('#Q2votes').text();
  var votes = parseInt(str);
  $('#Q2votes').text(votes-1)
}

function incQ3() {
  var str = $('#Q3votes').text();
  var votes = parseInt(str);
  $('#Q3votes').text(votes+1)
}
function decQ3() {
  var str = $('#Q3votes').text();
  var votes = parseInt(str);
  $('#Q3votes').text(votes-1)
}

function incH1() {
  var str = $('#H1votes').text();
  var votes = parseInt(str);
  $('#H1votes').text(votes+1)
}
function decH1() {
  var str = $('#H1votes').text();
  var votes = parseInt(str);
  $('#H1votes').text(votes-1)
}

function incH2() {
  var str = $('#H2votes').text();
  var votes = parseInt(str);
  $('#H2votes').text(votes+1)
}
function decH2() {
  var str = $('#H2votes').text();
  var votes = parseInt(str);
  $('#H2votes').text(votes-1)
}
