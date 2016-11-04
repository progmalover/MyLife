package com.titan.domain;

import javax.persistence.Entity;
import javax.persistence.Table;
import javax.persistence.Column;
import javax.persistence.Id;

/**
 * Overview
 * ========
 * This is Entity Bean.
 * 
 * The "@Entity" token talks to persistent provider that this class is a
 * database bean, who is controlled by EntityManager(???) sevice.
 * 
 * The "@Table" token talks to EJB Container(???) that this object of this
 * class need persistent.
 * 
 * This class implements java.io.Serializable, which make object could be
 * serialize to store in disk or translate on network.
 * 
 * 
 * Table-mapping Comments
 * ======================
 * Table-mapping comments like "@Table" "@Id" is not necessary. If omit these
 * comments, class name and member function name.
 * 
 * */

@Entity
@Table(name="CABIN")
public class Cabin implements java.io.Serializable 
{
    private int id;
    private String name;
    private int deckLevel;
    private int shipId;
    private int bedCount;

    @Id
    @Column(name="ID")
    public int getId()
    {
	return id;
    }
    public void setId(int pk) 
    {
	id = pk;
    }

    @Column(name="NAME")
    public String getName()
    {
        return name;
    }
    public void setName(String str)
    {
	name = str;
    }

    @Column(name="DECK_LEVEL")
    public int getDeckLevel() 
    {
	return deckLevel;
    }
    public void setDeckLevel(int level)
    {
	deckLevel = level;
    }

    @Column(name="SHIP_ID")
    public int getShipId()
    {
	return shipId;
    }
    public void setShipId(int sid)
    {
	shipId = sid;
    }

    @Column(name="BED_COUNT")
    public int getBedCount() 
    { 
	return bedCount; 
    }
    public void setBedCount(int bed) 
    { 
	bedCount = bed; 
    }
}

 

    
    

    
