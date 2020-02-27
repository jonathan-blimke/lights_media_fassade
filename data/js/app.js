// used when hosting the site somewhere other than the ESP8266 (handy for testing without waiting forever to upload to SPIFFS)
var address = "192.168.0.10";
var urlBase = "http://" + address + "/";
// console.debug("app.jssss");
var postColorTimer = {};
var postValueTimer = {}; 

var ignoreColorChange = false;

// var ws = new ReconnectingWebSocket('ws://' + address + ':81/', ['arduino']);
// ws.debug = true;

// ws.onmessage = function (evt) {
//   if (evt.data != null) {
//     var data = JSON.parse(evt.data);
//     if (data == null) return;  //unreachable if case?
//     updateFieldValue(data.name, data.value);
//   }
// }

$(document).ready(function () {
  $("#status").html("Connecting, please wait...");

  $.get(urlBase + "all", function (data) {
    $("#status").html("Loading, please wait...");
   
    $.each(data, function (index, field) {
      if (field.type == "Number") {
        addNumberField(field);
      } else if (field.type == "Boolean") {
        addBooleanField(field);
      }
      else if(field.type == "Text") {    
      
        addTextField(field);              
      }
   
    });
    $("#status").html("Ready");
  })
  .fail(function (jqXHR, textStatus, error) {
    console.log("Post error: " + error);
  });
});

function updateFieldValue(name, value) {
  var group = $("#form-group-" + name);

  var type = group.attr("data-field-type");

  if (type == "Number") {
    var input = group.find(".form-control");
    input.val(value);
  } else if (type == "Boolean") {
    var btnOn = group.find("#btnOn" + name);
    var btnOff = group.find("#btnOff" + name);
    btnOn.attr("class", value ? "btn btn-primary" : "btn btn-default");
    btnOff.attr("class", !value ? "btn btn-primary" : "btn btn-default");
  } else if(type = "Text") {
    var textinput = group.find(".form-control");
    input.val=(value);
  } //else if(type = "ArrayField") {
  // update bootstrap frontend with data from json
  //}

}

function addNumberField(field) {
  var template = $("#numberTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);

  var label = template.find(".control-label");
  label.attr("for", "input-" + field.name);
  label.text(field.label);

  var input = template.find(".input");
  var slider = template.find(".slider");
  slider.attr("id", "input-" + field.name);
  if (field.min) {
    input.attr("min", field.min);
    slider.attr("min", field.min);
  }
  if (field.max) {
    input.attr("max", field.max);
    slider.attr("max", field.max);
  }
  if (field.step) {
    input.attr("step", field.step);
    slider.attr("step", field.step);
  }
  input.val(field.value);
  slider.val(field.value);

  slider.on("change mousemove", function () {
    input.val($(this).val());
  });

  slider.on("change", function () {
    var value = $(this).val();
    input.val(value);
    field.value = value;
    delayPostValue(field.name, value);
  });

  input.on("change", function () {
    var value = $(this).val();
    slider.val(value);
    field.value = value;
    delayPostValue(field.name, value);
  });

  $("#form").append(template);
}

function addBooleanField(field) {
  var template = $("#booleanTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);

  var label = template.find(".control-label");
  label.attr("for", "btn-group-" + field.name);
  label.text(field.label);

  var btngroup = template.find(".btn-group");
  btngroup.attr("id", "btn-group-" + field.name);

  var btnOn = template.find("#btnOn");
  var btnOff = template.find("#btnOff");

  btnOn.attr("id", "btnOn" + field.name);
  btnOff.attr("id", "btnOff" + field.name);

  btnOn.attr("class", field.value ? "btn btn-primary" : "btn btn-default");
  btnOff.attr("class", !field.value ? "btn btn-primary" : "btn btn-default");

  btnOn.click(function () {
    setBooleanFieldValue(field, btnOn, btnOff, 1)
  });
  btnOff.click(function () {
    setBooleanFieldValue(field, btnOn, btnOff, 0)
  });

  $("#form").append(template);
}

function addTextField(field) {
  var template = $("#textTemplate").clone();
  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);

  var label = template.find(".control-label");
  label.attr("for", "input-" + field.name);
  label.text(field.label);

  var displaybtn = template.find("#btndisplay");
  displaybtn.attr("id", "btndisplay" + field.name);
  displaybtn.attr("class", "btn btn-primary");

  var input = template.find(".input");
  input.val(field.value);
  input.on("change", function() {
    console.debug("on change input");
    var value = $(this).val();
    setTextFieldValue(field, value);

    });
  displaybtn.click(function () {

  });

  $("#form").append(template);
}

function setBooleanFieldValue(field, btnOn, btnOff, value) {
  field.value = value;

  btnOn.attr("class", field.value ? "btn btn-primary" : "btn btn-default");
  btnOff.attr("class", !field.value ? "btn btn-primary" : "btn btn-default");

  postValue(field.name, field.value);
}

function setTextFieldValue(field, value) {
  field.value = value; 
  delayPostValue(field.name, field.value);
} 

function postValue(name, value) {
  $("#status").html("Setting " + name + ": " + value + ", please wait..."); 

  var body = {
    name: name,
    value: value
  };
//http://192.168.0.140/fieldValue?value=0&name=power
  $.post(urlBase + 'fieldValue' + "?value=" + value + '&name=' + name, body, function (data) {
    $("#status").html("Set " + name + ": " + value);
  });
}

function delayPostValue(name, value) {
  clearTimeout(postValueTimer);
  postValueTimer = setTimeout(function () {
    postValue(name, value);
  }, 300);
}
