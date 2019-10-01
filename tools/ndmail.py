# Import smtplib for the actual sending function
import os
import smtplib

# Import the email modules we'll need
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.utils import formataddr
from email.utils import formatdate

mail_host       = "smtp.newdimchina.com"
mail_user       = "WANG.Wei.1982"
mail_pass       = "8ik,0p;/"
mail_postfix    = "newdimchina.com"
me              = "Wei Wang"+"<"+mail_user+"@"+mail_postfix+">"

def addAttachment(attachPathName, msg):
    attaName = os.path.basename(attachPathName)
    with open(attachPathName, "rb") as fo:
        atta = MIMEText(fo.read(), "base64", "gb2312")
        atta["Content-Type"] = 'application/octet-stream'
        atta["Content-Disposition"] = 'attachment; filename="'+attaName+'"'
        msg.attach(atta)

def addHtmlContent(htmlPathName, msg):
    with open(htmlPathName) as fo:
        cont = MIMEText(fo.read(), _subtype="html", _charset="gb2312")
        msg.attach(cont)

def addPlainContent(plainText, msg):
    cont = MIMEText(plainText, _subtype='plain', _charset='gb2312')
    msg.attach(cont)

def sendHtmlMail(to_list, subj):
    me="Wei Wang"+"<"+mail_user+"@"+mail_postfix+">"
    msg=MIMEMultipart()
    msg["Subject"]  = subj
    msg["From"]     = me
    msg["To"]       = ";".join(to_list)
    #msg["Cc"]       = ";".join(["WU.MingShi@newdimchina.com"])
    #msg["Bcc"]      = ";".join(["CAO.Bingwan@newdimchina.com"])
    msg["Date"]     = formatdate()

    addHtmlContent(r"Quicklook\email_QL_template.html", msg)
    addAttachment(r"Quicklook\QL_REQUEST_template.xlsx", msg)

    #for k in msg.keys():
    #    print k, msg[k]
    to_addrs = to_list#+["WU.MingShi@newdimchina.com"]+["CAO.Bingwan@newdimchina.com"]

    try:
        server = smtplib.SMTP()
        server.connect(mail_host)
        server.login(mail_user, mail_pass)
        server.sendmail(me, to_addrs, msg.as_string())
    except Exception as e:
        print e.message
        return False
    finally:
        server.close()

    return True

def sendMail(from_addr, to_addrs, msg):
    try:
        server = smtplib.SMTP()
        server.connect(mail_host)
        server.login(mail_user, mail_pass)
        server.sendmail(from_addr, to_addrs, msg.as_string())
    except Exception as e:
        print e.message
        return False
    finally:
        server.close()

    return True

def prepareMsgHeader(msg, mailTo, mailCc, mailBcc, subj):
    msg['Subject']  = subj
    msg['From']     = me  
    msg['To']       = ";".join(mailTo)
    if mailCc: msg["Cc"] = ";".join(mailCc)
    if mailBcc: msg["Bcc"] = ";".join(mailBcc)
    msg["Date"]     = formatdate()

def sendPlainMail(mailTo, mailCc, mailBcc, subj, mainBody):
    msg = MIMEText(mainBody, _subtype='plain', _charset='gb2312')
    prepareMsgHeader(msg, mailTo, mailCc, mailBcc, subj)
    to_addrs = mailTo+mailCc+mailBcc

    return sendMail(me, to_addrs, msg)

def sendPlainAttachmentMail(mailTo, mailCc, mailBcc, subj, mainBody, attachments):
    msg=MIMEMultipart()
    prepareMsgHeader(msg, mailTo, mailCc, mailBcc, subj)
    addPlainContent(mainBody, msg)
    for att in attachments:
        addAttachment(att, msg)

    to_addrs = mailTo+mailCc+mailBcc
    return sendMail(me, to_addrs, msg)


if "__main__"==__name__:
    if sendPlainMail(["WANG.Wei.1982@newdimchina.com"], [], [], "Hello from python client", "This is a test mail"):
        print "send success"
    else:
        print "send fail"
