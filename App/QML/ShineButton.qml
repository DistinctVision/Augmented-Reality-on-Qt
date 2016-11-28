import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Button {
    id: button

    property real radius: height / 5

    property real lineWidth: 3.0
    property string strokeStyle: "#0088ff"
    property string fillStyle: "#0088ff"
    property color textColor: button.hovered && !button.pressed ? "white" : "#ddd"
    property int fontPointSize: 16
    property int minimumPixelSize: 2
    property int fontSizeMode: Text.Fit
    opacity: !button.pressed ? 1 : 0.75

    style: ButtonStyle {
        background: Item {
            Canvas {
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();

                    ctx.beginPath();
                    ctx.lineWidth = button.lineWidth;
                    ctx.roundedRect(ctx.lineWidth / 2, ctx.lineWidth / 2,
                        width - ctx.lineWidth, height - ctx.lineWidth, button.radius, button.radius);
                    ctx.strokeStyle = strokeStyle;
                    ctx.stroke();
                    ctx.fillStyle = fillStyle;
                    ctx.fill();
                }
            }
            Label {
                anchors.centerIn: parent
                anchors.fill: parent
                text: button.text
                color: textColor
                fontSizeMode: fontSizeMode
                minimumPixelSize: minimumPixelSize
                font.pointSize: fontPointSize
                font.bold: true
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();

                    ctx.beginPath();
                    ctx.lineWidth = button.lineWidth;
                    ctx.roundedRect(ctx.lineWidth / 2, ctx.lineWidth / 2,
                        width - ctx.lineWidth, height - ctx.lineWidth, button.radius, button.radius);
                    ctx.moveTo(0, height * 0.4);
                    ctx.bezierCurveTo(width * 0.25, height * 0.6, width * 0.75, height * 0.6, width, height * 0.4);
                    ctx.lineTo(width, height);
                    ctx.lineTo(0, height);
                    ctx.lineTo(0, height * 0.4);
                    ctx.clip();

                    ctx.beginPath();
                    ctx.roundedRect(ctx.lineWidth / 2, ctx.lineWidth / 2,
                        width - ctx.lineWidth, height - ctx.lineWidth,
                        button.radius, button.radius);
                    var gradient = ctx.createLinearGradient(0, 0, 0, height);
                    gradient.addColorStop(0, "#bbffffff");
                    gradient.addColorStop(0.6, "#00ffffff");
                    ctx.fillStyle = gradient;
                    ctx.fill();
                }
            }
        }

        label: null
    }
}
