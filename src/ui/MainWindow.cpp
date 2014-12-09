//
//  MainWindow.cpp
//  StackManagerQt/src/ui
//
//  Created by Mohammed Nafees on 10/17/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "MainWindow.h"

#include <QClipboard>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMutex>
#include <QLayoutItem>
#include <QCursor>
#include <QtWebKitWidgets/qwebview.h>

#include "AppDelegate.h"
#include "AssignmentWidget.h"
#include "GlobalData.h"

const int GLOBAL_X_PADDING = 55;
const int TOP_Y_PADDING = 25;
const int REQUIREMENTS_TEXT_TOP_MARGIN = 19;
const int HORIZONTAL_RULE_TOP_MARGIN = 25;

const int BUTTON_PADDING_FIX = -5;

const int ASSIGNMENT_LAYOUT_RESIZE_FACTOR = 56;
const int ASSIGNMENT_LAYOUT_WIDGET_STRETCH = 0;
const QColor lightGrayColor = QColor(205, 205, 205);
const QColor darkGrayColor = QColor(84, 84, 84);
const QColor redColor = QColor(189, 54, 78);
const QColor greenColor = QColor(3, 150, 126);

MainWindow* MainWindow::_instance = NULL;

MainWindow* MainWindow::getInstance() {
    static QMutex instanceMutex;

    instanceMutex.lock();

    if (!_instance) {
        _instance = new MainWindow();
    }

    instanceMutex.unlock();

    return _instance;
}

const QString SHARE_BUTTON_SHARE_TEXT = "Share";
const QString SHARE_BUTTON_COPY_LINK_TEXT = "Copy link";
const QString SHARE_BUTTON_REQUESTING_TEXT = "Requesting...";

MainWindow::MainWindow() :
    QWidget(0),
    _domainServerRunning(false),
    _startServerButton(NULL),
    _stopServerButton(NULL),
    _serverAddressLabel(NULL),
    _viewLogsButton(NULL),
    _settingsButton(NULL),
    _runAssignmentButton(NULL),
    _shareButton(NULL),
    _contentSetButton(NULL),
    _logsWidget(NULL),
    _assignmentLayout(NULL),
    _assignmentScrollArea(NULL)
{
    setWindowTitle("High Fidelity Stack Manager");
    const int WINDOW_FIXED_WIDTH = 640;
    const int WINDOW_INITIAL_HEIGHT = 200;

    if (GlobalData::getInstance()->getPlatform() == "win") {
        const int windowsYCoord = 30;
        setGeometry(qApp->desktop()->availableGeometry().width() / 2 - WINDOW_FIXED_WIDTH / 2, windowsYCoord,
                    WINDOW_FIXED_WIDTH, WINDOW_INITIAL_HEIGHT);
    } else {
        const int unixYCoord = 0;
        setGeometry(qApp->desktop()->availableGeometry().width() / 2 - WINDOW_FIXED_WIDTH / 2, unixYCoord,
                    WINDOW_FIXED_WIDTH, WINDOW_INITIAL_HEIGHT);
    }
    setFixedWidth(WINDOW_FIXED_WIDTH);
    setMaximumHeight(qApp->desktop()->availableGeometry().height());
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                   Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setMouseTracking(true);
    setStyleSheet("font-family: 'Helvetica', 'Arial', 'sans-serif';");

    const int SERVER_BUTTON_HEIGHT = 47;
    
    _startServerButton = new SvgButton(this);
    
    QPixmap scaledStart(":/server-start.svg");
    scaledStart.scaledToHeight(SERVER_BUTTON_HEIGHT);
    
    _startServerButton->setGeometry((width() / 2.0f) - (scaledStart.width() / 2.0f),
                                    TOP_Y_PADDING,
                                    scaledStart.width(),
                                    scaledStart.height());
    _startServerButton->setSvgImage(":/server-start.svg");
    
    _stopServerButton = new SvgButton(this);
    _stopServerButton->setSvgImage(":/server-stop.svg");
    _stopServerButton->setGeometry(GLOBAL_X_PADDING, TOP_Y_PADDING,
                                   scaledStart.width(), scaledStart.height());

    const int SERVER_ADDRESS_LABEL_LEFT_MARGIN = 20;
    const int SERVER_ADDRESS_LABEL_TOP_MARGIN = 17;
    _serverAddressLabel = new QLabel(this);
    _serverAddressLabel->move(_stopServerButton->geometry().right() + SERVER_ADDRESS_LABEL_LEFT_MARGIN,
                              TOP_Y_PADDING + SERVER_ADDRESS_LABEL_TOP_MARGIN);
    _serverAddressLabel->setOpenExternalLinks(true);

    const int SECONDARY_BUTTON_ROW_TOP_MARGIN = 10;
    
    int secondaryButtonY = _stopServerButton->geometry().bottom() + SECONDARY_BUTTON_ROW_TOP_MARGIN;
    
    _viewLogsButton = new QPushButton("View logs", this);
    _viewLogsButton->adjustSize();
    _viewLogsButton->setGeometry(GLOBAL_X_PADDING + BUTTON_PADDING_FIX, secondaryButtonY,
                                 _viewLogsButton->width(), _viewLogsButton->height());

    _settingsButton = new QPushButton("Settings", this);
    _settingsButton->adjustSize();
    _settingsButton->setGeometry(_viewLogsButton->geometry().right(), secondaryButtonY,
                                 _settingsButton->width(), _settingsButton->height());
    
    _shareButton = new QPushButton(SHARE_BUTTON_COPY_LINK_TEXT, this);
    _shareButton->adjustSize();
    _shareButton->setGeometry(_settingsButton->geometry().right(), secondaryButtonY,
                              _shareButton->width(), _shareButton->height());
    
    // add the drop down for content sets
    _contentSetButton = new QPushButton("Get content set", this);
    _contentSetButton->adjustSize();
    _contentSetButton->setGeometry(_shareButton->geometry().right(), secondaryButtonY,
                                   _contentSetButton->width(), _contentSetButton->height());
    
    const int ASSIGNMENT_BUTTON_TOP_MARGIN = 10;

    _runAssignmentButton = new QPushButton("Run assignment", this);
    _runAssignmentButton->move(GLOBAL_X_PADDING + BUTTON_PADDING_FIX,
                               _viewLogsButton->geometry().bottom() + REQUIREMENTS_TEXT_TOP_MARGIN
                               + HORIZONTAL_RULE_TOP_MARGIN + ASSIGNMENT_BUTTON_TOP_MARGIN);

    const QSize logsWidgetSize = QSize(500, 500);
    _logsWidget = new QTabWidget;
    _logsWidget->setUsesScrollButtons(true);
    _logsWidget->setElideMode(Qt::ElideMiddle);
    _logsWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                                Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    _logsWidget->resize(logsWidgetSize);
    
    const int ASSIGNMENT_SCROLL_AREA_TOP_MARGIN = 10;

    _assignmentScrollArea = new QScrollArea(this);
    _assignmentScrollArea->setWidget(new QWidget);
    _assignmentScrollArea->setWidgetResizable(true);
    _assignmentScrollArea->setFrameShape(QFrame::NoFrame);
    _assignmentScrollArea->move(GLOBAL_X_PADDING, _runAssignmentButton->geometry().bottom() + ASSIGNMENT_SCROLL_AREA_TOP_MARGIN);
    _assignmentScrollArea->setMaximumWidth(width() - GLOBAL_X_PADDING * 2);
    _assignmentScrollArea->setMaximumHeight(qApp->desktop()->availableGeometry().height() -
                                            _assignmentScrollArea->geometry().top());

    const int assignmentLayoutSpacingMargin = 0;
    _assignmentLayout = new QVBoxLayout;
    _assignmentLayout->setSpacing(assignmentLayoutSpacingMargin);
    _assignmentLayout->setMargin(assignmentLayoutSpacingMargin);
    _assignmentLayout->setContentsMargins(assignmentLayoutSpacingMargin, assignmentLayoutSpacingMargin,
                                          assignmentLayoutSpacingMargin, assignmentLayoutSpacingMargin);
    _assignmentScrollArea->widget()->setLayout(_assignmentLayout);

    connect(_startServerButton, &QPushButton::clicked, this, &MainWindow::toggleDomainServerButton);
    connect(_stopServerButton, &QPushButton::clicked, this, &MainWindow::toggleDomainServerButton);
    connect(_shareButton, &QPushButton::clicked, this, &MainWindow::handleShareButton);
    connect(_contentSetButton, &QPushButton::clicked, this, &MainWindow::showContentSetPage);
    connect(_viewLogsButton, &QPushButton::clicked, _logsWidget, &QTabWidget::show);
    connect(_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(_runAssignmentButton, &QPushButton::clicked, this, &MainWindow::addAssignment);
    
    AppDelegate* app = AppDelegate::getInstance();
    // update the current server address label and change it if the AppDelegate says the address has changed
    updateServerAddressLabel();
    connect(app, &AppDelegate::domainAddressChanged, this, &MainWindow::updateServerAddressLabel);
    
    // if domain is missing an ID, let us switch our share button text
    connect(app, &AppDelegate::domainServerIDMissing, this, &MainWindow::toggleShareButtonText);
    
    // handle temp domain response for window
    connect(app, &AppDelegate::temporaryDomainResponse, this, &MainWindow::handleTemporaryDomainCreateResponse);
    
    // handle response for content set download
    connect(app, &AppDelegate::contentSetDownloadResponse, this, &MainWindow::handleContentSetDownloadResponse);
    
    toggleContent(false);

}

void MainWindow::updateServerAddressLabel() {
    AppDelegate* app = AppDelegate::getInstance();
    
    _serverAddressLabel->setText("<html><head/><body style=\"font:14px 'Helvetica', 'Arial', 'sans-serif';"
                                 "font-weight: bold;\"><p><span style=\"color:#545454;\">Accessible at: </span>"
                                 "<a href=\"" + app->getServerAddress() + "\">"
                                 "<span style=\"color:#29957e;\">" + app->getServerAddress(false) +
                                 "</span></a></p></body></html>");
    _serverAddressLabel->adjustSize();
    
    _shareButton->setText(SHARE_BUTTON_COPY_LINK_TEXT);
    _shareButton->setEnabled(true);
}

void MainWindow::toggleShareButtonText() {
    _shareButton->setText(_shareButton->text() == SHARE_BUTTON_COPY_LINK_TEXT
                          ? SHARE_BUTTON_SHARE_TEXT : SHARE_BUTTON_COPY_LINK_TEXT);
}

void MainWindow::handleShareButton() {
    if (_shareButton->text() == SHARE_BUTTON_COPY_LINK_TEXT) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(AppDelegate::getInstance()->getServerAddress());
    } else {
        // user hit the share button, show them a dialog asking them if they want to get a temp name
        const QString SHARE_DIALOG_MESSAGE = "This will create a temporary domain name (valid for 30 days)"
            " so other users can easily connect to your domain.\n\nThis will restart your domain with"
            " the new temporary name and ID.\n\nDo you want to continue?";
        QMessageBox::StandardButton clickedButton = QMessageBox::question(this, "Share domain",  SHARE_DIALOG_MESSAGE);
        
        if (clickedButton == QMessageBox::Yes) {
            _shareButton->setText(SHARE_BUTTON_REQUESTING_TEXT);
            _shareButton->setEnabled(false);
            
            AppDelegate::getInstance()->requestTemporaryDomain();
        }
    }
}

void MainWindow::showContentSetPage() {
    const QString CONTENT_SET_HTML_URL = "http://hifi-public.s3.amazonaws.com/content-sets/content-sets.html";
    
    // show a QWebView for the content set page
    QWebView* contentSetWebView = new QWebView();
    contentSetWebView->setUrl(CONTENT_SET_HTML_URL);
    
    // have the widget delete on close
    contentSetWebView->setAttribute(Qt::WA_DeleteOnClose);
    
    // setup the page viewport to be the right size
    const QSize CONTENT_SET_VIEWPORT_SIZE = QSize(800, 400);
    contentSetWebView->resize(CONTENT_SET_VIEWPORT_SIZE);
    
    // have our app delegate handle a click on one of the content sets
    contentSetWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(contentSetWebView->page(), &QWebPage::linkClicked, AppDelegate::getInstance(), &AppDelegate::downloadContentSet);
    connect(contentSetWebView->page(), &QWebPage::linkClicked, contentSetWebView, &QWebView::close);
    
    contentSetWebView->show();
}

void MainWindow::handleTemporaryDomainCreateResponse(bool wasSuccessful) {
    if (wasSuccessful) {
        _shareButton->setEnabled(true);
        _shareButton->setText(SHARE_BUTTON_COPY_LINK_TEXT);
    } else {
        _shareButton->setEnabled(true);
        _shareButton->setText(SHARE_BUTTON_SHARE_TEXT);
        
        QMessageBox::information(this, "Error", "There was a problem sharing your domain. Please try again!");
    }
}

void MainWindow::handleContentSetDownloadResponse(bool wasSuccessful) {
    if (wasSuccessful) {
        QMessageBox::information(this, "New content set",
                                 "Your new content set has been downloaded and your assignment-clients have been restarted.");
    } else {
        QMessageBox::information(this, "Error", "There was a problem downloading that content set. Please try again!");
    }
}

void MainWindow::setRequirementsLastChecked(const QString& lastCheckedDateTime) {
    _requirementsLastCheckedDateTime = lastCheckedDateTime;
}

void MainWindow::toggleContent(bool isRunning) {
    _stopServerButton->setVisible(isRunning);
    _startServerButton->setVisible(!isRunning);
    _domainServerRunning = isRunning;
    _serverAddressLabel->setVisible(isRunning);
    _viewLogsButton->setVisible(isRunning);
    _settingsButton->setVisible(isRunning);
    _shareButton->setVisible(isRunning);
    _contentSetButton->setVisible(isRunning);
    _runAssignmentButton->setVisible(isRunning);
    _assignmentScrollArea->setVisible(isRunning);
    _assignmentScrollArea->widget()->setEnabled(isRunning);
    update();
}

void MainWindow::setDomainServerStarted() {
    toggleContent(true);
}

void MainWindow::setDomainServerStopped() {
    toggleContent(false);
    
    for (int i = 0; i < _assignmentLayout->count(); ++i) {
        if (qobject_cast<AssignmentWidget*>(_assignmentLayout->itemAt(i)->widget())->isRunning()) {
            qobject_cast<AssignmentWidget*>(_assignmentLayout->itemAt(i)->widget())->toggleRunningState();
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font("Helvetica");
    font.insertSubstitutions("Helvetica", QStringList() << "Arial" << "sans-serif");
    
    int currentY = (_domainServerRunning ?  _viewLogsButton->geometry().bottom() : _startServerButton->geometry().bottom())
        + REQUIREMENTS_TEXT_TOP_MARGIN;
    
    if (!_requirementsLastCheckedDateTime.isEmpty()) {
        font.setBold(false);
        font.setUnderline(false);
        painter.setFont(font);
        painter.setPen(darkGrayColor);
        
        QString requirementsString = "Requirements are up to date as of " + _requirementsLastCheckedDateTime;
        float fontWidth = QFontMetrics(font).width(requirementsString);
        
        painter.drawText(QRectF(_domainServerRunning ? GLOBAL_X_PADDING : ((width() - fontWidth)/ 2.0f),
                                currentY,
                                fontWidth,
                                QFontMetrics(font).height()),
                         "Requirements are up to date as of " + _requirementsLastCheckedDateTime);
    }
    
    if (_domainServerRunning) {
        currentY += HORIZONTAL_RULE_TOP_MARGIN;
        painter.setPen(lightGrayColor);
        painter.drawLine(0, currentY, width(), currentY);
    }
}

void MainWindow::toggleDomainServerButton() {
    if (_domainServerRunning) {
        AppDelegate::getInstance()->stopDomainServer();
    } else {
        AppDelegate::getInstance()->startDomainServer();
    }
}

void MainWindow::addAssignment() {
    AssignmentWidget* widget = new AssignmentWidget(_assignmentLayout->count() + 1);
    _assignmentLayout->addWidget(widget, ASSIGNMENT_LAYOUT_WIDGET_STRETCH, Qt::AlignTop);
    resize(width(), _assignmentScrollArea->geometry().y()
           + ASSIGNMENT_LAYOUT_RESIZE_FACTOR * _assignmentLayout->count()
           + TOP_Y_PADDING);
    _assignmentScrollArea->resize(_assignmentScrollArea->maximumWidth(), height() - _assignmentScrollArea->geometry().top());
}

void MainWindow::openSettings() {
    QDesktopServices::openUrl(QUrl(DOMAIN_SERVER_BASE_URL + "/settings/"));
}
