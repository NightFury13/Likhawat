# -*- coding: utf-8 -*-
# this file is released under public domain and you can use without limitations

#########################################################################
## This is a sample controller
## - index is the default action of any application
## - user is required for authentication and authorization
## - download is for downloading files uploaded in the db (does streaming)
## - api is an example of Hypermedia API support and access control
#########################################################################

import os
import commands

def index():
    """
    example action using the internationalization operator T and flash
    rendered by views/default/index.html or views/generic.html

    if you need a simple wiki simply replace the two lines below with:
    return auth.wiki()
    """
    response.flash = T("Welcome to Likhawat!")
    return dict(message=T('Handwritten Document Analyser'))


def user():
    """
    exposes:
    http://..../[app]/default/user/login
    http://..../[app]/default/user/logout
    http://..../[app]/default/user/register
    http://..../[app]/default/user/profile
    http://..../[app]/default/user/retrieve_password
    http://..../[app]/default/user/change_password
    http://..../[app]/default/user/manage_users (requires membership in
    use @auth.requires_login()
        @auth.requires_membership('group name')
        @auth.requires_permission('read','table name',record_id)
    to decorate functions that need access control
    """
    return dict(form=auth())

@auth.requires_login()
def segmenter_homepage():
    """
    The first page of segmenter stream showing all Documents on the database and option to add a new Document.
    """
    # beautification variables
    message = T('Select the image to be segmented :')
    # offset to show the images in an ordered fashion
    offset = 0
    if request.vars['offset']:
        offset = int(request.vars['offset'])

    displayDocuments = db((db.Documents.id>offset) & (db.Documents.id<offset+15)).select(db.Documents.ALL)
    docUploadForm = SQLFORM(db.Documents)
    if docUploadForm.process().accepted:
        response.flash = 'File Successfully Uploaded!'
    elif docUploadForm.process().errors:
        response.flash = 'Error Uploading the file!'

    return locals()

@auth.requires_login()
def segmented_images():
    """
    This page shows all the segments that are found by the 'segmenter' code.
    Runs the nogl.cpp file with input argument passed along with the image as variable 'parentImage'
    """
    # beatufication variables
    message = T('Here are the segments of the image...')
    if request.vars['parentImage']:
        parentImage = request.vars['parentImage']
    else:
        response.flash = 'Error! No source image found!'
        # redirect(URL(r=request, f='segmenter_homepage'))

    # Segment the given image.
    try:
        os.system('bash ./applications/Likhawat/segmenter/segment.sh') 
    except:
        response.flash = "Failed to run the segmentation bot! :("

    return locals()    
 
@cache.action()
def download():
    """
    allows downloading of uploaded files
    http://..../[app]/default/download/[filename]
    """
    return response.download(request, db)


def call():
    """
    exposes services. for example:
    http://..../[app]/default/call/jsonrpc
    decorate with @services.jsonrpc the functions to expose
    supports xml, json, xmlrpc, jsonrpc, amfrpc, rss, csv
    """
    return service()


@auth.requires_login() 
def api():
    """
    this is example of API with access control
    WEB2PY provides Hypermedia API (Collection+JSON) Experimental
    """
    from gluon.contrib.hypermedia import Collection
    rules = {
        '<tablename>': {'GET':{},'POST':{},'PUT':{},'DELETE':{}},
        }
    return Collection(db).process(request,response,rules)