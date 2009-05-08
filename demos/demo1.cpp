/***************************************************************************
 *   Copyright 2005-2009 Last.fm Ltd.                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include <lastfm.h> // this includes everything in liblastfm, you may prefer 
#include <QtCore>   // to just include what you need with your project. Still
#include <QtGui>    // we've given you the option.
#include <QPointer>
#include <QNetworkReply>

class ArtistList : public QListWidget
{
    Q_OBJECT
    
    QPointer<QNetworkReply> reply;
    QString artist;
    
public:
    ArtistList()
    {
        connect( this,
                 SIGNAL(itemActivated( QListWidgetItem* )),
                 SLOT(onItemActivated( QListWidgetItem* )) );
    }
    
    void getSimilar( const QString& artist )
    {
        this->artist = artist;
        setWindowTitle( "Loading " + artist + "..." );
        
        // deleting a reply cancels the request and disconnects all signals
        delete reply;
        reply = lastfm::Artist( artist ).getSimilar();
        connect( reply, SIGNAL(finished()), SLOT(onGotSimilar()) );
    }
    
private slots:
    void onGotSimilar()
    {
        QNetworkReply* r = static_cast<QNetworkReply*>(sender());
        // always enclose retrieval functions in a try block, as they will
        // throw if they can't parse the data
        try
        {
            // you decode the response using the equivalent static function
            QMap<int, QString> artists = lastfm::Artist::getSimilar( r );

            clear();

            // we iterate backwards because best match is last because the map
            // sorts itself by key
            QStringListIterator i( artists.values() );
            i.toBack();
            while (i.hasPrevious())
                addItem( i.previous() );
                
            setWindowTitle( artist );
        }
        catch (std::runtime_error& e)
        {
            // if getSimilar() failed to parse the QNetworkReply, then e will 
            // be of type lastfm::ws::ParseError, which derives 
            // std::runtime_error
            qWarning() << e.what();
        }
    }
    
    void onItemActivated( QListWidgetItem* item )
    {
        getSimilar( item->text() );
    }
};


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    app.setApplicationName( "liblastfm" ); // used to generate UserAgent
    
    // all you need for non-authenticated webservices is your API key
    // this one is a public one, it can only do artist.getSimilar calls, so
    // I suggest you don't use it :P
    lastfm::ws::ApiKey = "b25b959554ed76058ac220b7b2e0a026";

    ArtistList artists;
    artists.getSimilar( "nirvana" );
    artists.resize( 300, 400 ); // Qt picks truly asanine default sizes for its widgets
    artists.show();

    return app.exec();
}


#include "demo1.moc"
